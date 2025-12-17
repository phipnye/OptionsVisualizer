#include "BS_thread_pool.hpp"
#include "OptionsVisualizer/Grid/Enums.hpp"
#include "OptionsVisualizer/Grid/Grid.hpp"
#include "OptionsVisualizer/models/trinomial/calculate_price.hpp"
#include "OptionsVisualizer/models/trinomial/constants.hpp"
#include <array>
#include <future>
#include <iostream>
#include <unordered_map>
#include <unsupported/Eigen/CXX11/Tensor>
#include <utility>
#include <vector>

Grid::GreeksResult Grid::trinomialGreeks(OptionType optType) const {
    using models::trinomial::calculatePrice;

    // Construct a thread pool with as many threads as are available in the hardware
    static BS::thread_pool pool{};

    // Use proportional epsilons to stabilize greeks across discrete grids
    const double hSpot{spot_ * 0.05};
    constexpr double hSigma{0.01}; // Standard 1% absolute shift for volatility
    const double hTau{tau_ * 0.01};

    // --- Compute prices

    // Define a simple struct to hold small perturbations for spot, sigma, and tau
    struct Perturb {
        double dSpot{0.0};  // perturbation in spot price
        double dSigma{0.0}; // perturbation in volatility (sigma)
        double dTau{0.0};   // perturbation in time to expiry (tau)
        enum class Idx { Base, LoSpot, HiSpot, LoSigma, HiSigma, LoTau, HiTau } idx;
    };

    // List of perturbations to compute finite-difference approximations
    const std::array<Perturb, 7> perturbations{
        Perturb{0.0, 0.0, 0.0, Perturb::Idx::Base},        // base price (no perturbation)
        Perturb{-hSpot, 0.0, 0.0, Perturb::Idx::LoSpot},   // lower spot for delta
        Perturb{hSpot, 0.0, 0.0, Perturb::Idx::HiSpot},    // higher spot for delta
        Perturb{0.0, -hSigma, 0.0, Perturb::Idx::LoSigma}, // lower sigma for vega
        Perturb{0.0, hSigma, 0.0, Perturb::Idx::HiSigma},  // higher sigma for vega
        Perturb{0.0, 0.0, -hTau, Perturb::Idx::LoTau},     // lower tau for theta
        Perturb{0.0, 0.0, hTau, Perturb::Idx::HiTau}       // higher tau for theta
    };

    // Prepare a vector to hold futures for asynchronous price calculations
    std::vector<std::future<Eigen::Tensor<double, 2>>> futures{};
    futures.reserve(perturbations.size());

    // Launch asynchronous tasks for each perturbation using the thread pool
    for (const Perturb& p : perturbations) {
        futures.emplace_back(pool.submit_task([p, this, optType]() {
            // Compute the option price with the specified perturbation applied
            return calculatePrice(this->spot_ + p.dSpot, // perturbed spot
                                  this->strikesGrid_, this->r_, this->q_,
                                  this->sigmasGrid_ + p.dSigma, // perturbed sigmas
                                  this->tau_ + p.dTau,          // perturbed time
                                  this->nSigma_, this->nStrike_, optType);
        }));
    }

    // Retrieve the computed prices from the futures
    std::unordered_map<Perturb::Idx, Eigen::Tensor<double, 2>> prices{};
    prices.reserve(perturbations.size());

    for (std::size_t perturbIdx{0}; perturbIdx < perturbations.size(); ++perturbIdx) {
        prices[perturbations[perturbIdx].idx] = futures[perturbIdx].get();
    }

    // --- First-order derivatives (delta, vega, theta)

    static auto firstOrderCDM{[](const Eigen::Tensor<double, 2>& lo, const Eigen::Tensor<double, 2>& hi, double eps) {
        return (hi - lo) / (2.0 * eps);
    }};

    // Compute delta (first derivative w.r.t spot) using CDM
    // delta = (price(spot + dSpot) - price(spot - dSpot)) / (2 * dSpot)
    Eigen::Tensor<double, 2> delta{firstOrderCDM(prices[Perturb::Idx::LoSpot], prices[Perturb::Idx::HiSpot], hSpot)};

    // Compute vega (first derivative w.r.t sigma) using CDM
    // vega = (price(sigma + dSigma) - price(sigma - dSigma)) / (2 * dSigma)
    Eigen::Tensor<double, 2> vega{firstOrderCDM(prices[Perturb::Idx::LoSigma], prices[Perturb::Idx::HiSigma], hSigma)};

    // Compute theta (negative first derivative w.r.t tau) using CDM
    // theta = -(price(tau + dTau) - price(tau - dTau)) / (2 * dTau)
    Eigen::Tensor<double, 2> theta{-firstOrderCDM(prices[Perturb::Idx::LoTau], prices[Perturb::Idx::HiTau], hTau)};

    // --- Second-order derivatives (gamma)

    // Compute gamma (second derivative w.r.t spot) using CDM
    // gamma = (price(spot + dSpot) - 2 * price(spot) + price(spot - dSpot)) / (dSpot ^ 2)
    Eigen::Tensor<double, 2> gamma{
        (prices[Perturb::Idx::HiSpot] - (2.0 * prices[Perturb::Idx::Base]) + prices[Perturb::Idx::LoSpot]) /
        (hSpot * hSpot)};

    return GreeksResult{std::move(prices[Perturb::Idx::Base]), std::move(delta), std::move(gamma), std::move(vega),
                        std::move(theta)};
}
