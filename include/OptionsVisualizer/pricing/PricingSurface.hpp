#pragma once

#include <Eigen/Dense>
#include <array>
#include <future>
#include <utility>
#include <vector>

#include "BS_thread_pool.hpp"
#include "OptionsVisualizer/core/Enums.hpp"
#include "OptionsVisualizer/core/globals.hpp"
#include "OptionsVisualizer/models/trinomial/internal/calculate_price.hpp"
#include "OptionsVisualizer/pricing/GreeksResult.hpp"

class PricingSurface {
  // --- Data-members
  const Eigen::DenseIndex nSigma_;
  const Eigen::DenseIndex nStrike_;
  const double spot_;
  const double r_;
  const double q_;
  const Eigen::MatrixXd sigmasGrid_;
  const Eigen::MatrixXd strikesGrid_;
  const double tau_;
  BS::thread_pool<>& pool_;

 public:
  PricingSurface(Eigen::DenseIndex nSigma, Eigen::DenseIndex nStrike,
                 double spot, double r, double q, double sigmaLo,
                 double sigmaHi, double strikeLo, double strikeHi, double tau,
                 BS::thread_pool<>& pool);

  // Compute grids for all greeks and option types
  [[nodiscard]] std::array<Eigen::MatrixXd, globals::nGrids> calculateGrids()
      const;

 private:
  // --- Black-Scholes-Merton
  [[nodiscard]] GreeksResult bsmCallGreeks() const;

  [[nodiscard]] GreeksResult bsmPutGreeks(
      const GreeksResult& callResults) const;

  // --- Trinomial tree
  template <Enums::OptionType T>
  [[nodiscard]] GreeksResult trinomialGreeks() const {
    static_assert(
        T == Enums::OptionType::AmerCall || T == Enums::OptionType::AmerPut,
        "Trinomial greeks calculation only expected for American options");

    // Use proportional epsilons to stabilize greeks across discrete grids
    const double hSpot{spot_ * 0.05};
    constexpr double hSigma{0.01};  // Standard 1% absolute shift for volatility
    const double hTau{tau_ * 0.01};
    constexpr double hRho{
        0.01};  // 100 basis points absolute shift for risk-free rate

    // --- Compute prices

    // Define a simple struct to hold small perturbations for spot, sigma, and
    // tau
    struct Perturb {
      double dSpot;   // perturbation in spot price
      double dSigma;  // perturbation in volatility (sigma)
      double dTau;    // perturbation in time to expiry (tau)
      double dRho;    // perturbation in risk-free rate (rho)
      enum class Idx : std::size_t {
        Base,
        LoSpot,
        HiSpot,
        LoSigma,
        HiSigma,
        LoTau,
        HiTau,
        LoRho,
        HiRho,
        Count
      } idx;

      static constexpr auto getIdx(Idx idx) {
        return static_cast<std::underlying_type_t<Idx>>(idx);
      }
    };

    // List of perturbations to compute finite-difference approximations
    const std::array<Perturb, 9> perturbations{
        Perturb{0.0, 0.0, 0.0, 0.0,
                Perturb::Idx::Base},  // base price (no perturbation)
        Perturb{-hSpot, 0.0, 0.0, 0.0,
                Perturb::Idx::LoSpot},  // lower spot for delta
        Perturb{hSpot, 0.0, 0.0, 0.0,
                Perturb::Idx::HiSpot},  // higher spot for delta
        Perturb{0.0, -hSigma, 0.0, 0.0,
                Perturb::Idx::LoSigma},  // lower sigma for vega
        Perturb{0.0, hSigma, 0.0, 0.0,
                Perturb::Idx::HiSigma},  // higher sigma for vega
        Perturb{0.0, 0.0, -hTau, 0.0,
                Perturb::Idx::LoTau},  // lower tau for theta
        Perturb{0.0, 0.0, hTau, 0.0,
                Perturb::Idx::HiTau},  // higher tau for theta
        Perturb{0.0, 0.0, 0.0, -hRho, Perturb::Idx::LoRho},  // lower r for rho
        Perturb{0.0, 0.0, 0.0, hRho, Perturb::Idx::HiRho}    // higher r for rho
    };

    // Prepare a vector to hold futures for asynchronous price calculations
    std::vector<std::future<Eigen::MatrixXd>> futures{};
    futures.reserve(perturbations.size());

    // Launch asynchronous tasks for each perturbation using the thread pool
    for (const Perturb& p : perturbations) {
      futures.emplace_back(pool_.submit_task([p, this]() {
        // Compute the option price with the specified perturbation applied
        return models::trinomial::calculatePrice<T>(
            this->nSigma_, this->nStrike_,
            this->spot_ + p.dSpot,  // perturbed spot
            this->r_ + p.dRho,      // perturbed risk-free rate
            this->q_,
            this->sigmasGrid_.array() + p.dSigma,  // perturbed sigmas
            this->strikesGrid_,
            this->tau_ + p.dTau);  // perturbed time
      }));
    }

    // Retrieve the computed prices from the futures
    std::vector<Eigen::MatrixXd> prices(Perturb::getIdx(Perturb::Idx::Count));

    for (std::size_t perturbIdx{0}; perturbIdx < perturbations.size();
         ++perturbIdx) {
      prices[Perturb::getIdx(perturbations[perturbIdx].idx)] =
          futures[perturbIdx].get();
    }

    // --- First-order derivatives (delta, vega, theta, rho)

    const auto firstOrderCDM{
        [](const Eigen::MatrixXd& lo, const Eigen::MatrixXd& hi,
           const double eps) { return (hi - lo) / (2.0 * eps); }};

    // Compute delta (first derivative w.r.t spot) using CDM
    // delta = (price(spot + dSpot) - price(spot - dSpot)) / (2 * dSpot)
    Eigen::MatrixXd delta{
        firstOrderCDM(prices[Perturb::getIdx(Perturb::Idx::LoSpot)],
                      prices[Perturb::getIdx(Perturb::Idx::HiSpot)], hSpot)};

    // Compute vega (first derivative w.r.t sigma) using CDM
    // vega = (price(sigma + dSigma) - price(sigma - dSigma)) / (2 * dSigma)
    Eigen::MatrixXd vega{
        firstOrderCDM(prices[Perturb::getIdx(Perturb::Idx::LoSigma)],
                      prices[Perturb::getIdx(Perturb::Idx::HiSigma)], hSigma)};

    // Compute theta (negative first derivative w.r.t tau) using CDM
    // theta = -(price(tau + dTau) - price(tau - dTau)) / (2 * dTau)
    Eigen::MatrixXd theta{
        -firstOrderCDM(prices[Perturb::getIdx(Perturb::Idx::LoTau)],
                       prices[Perturb::getIdx(Perturb::Idx::HiTau)], hTau)};

    // Compute rho (first derivate w.r.t r) using CDM
    Eigen::MatrixXd rho{
        firstOrderCDM(prices[Perturb::getIdx(Perturb::Idx::LoRho)],
                      prices[Perturb::getIdx(Perturb::Idx::HiRho)], hRho)};

    // --- Second-order derivatives (gamma)

    // Compute gamma (second derivative w.r.t spot) using CDM
    // gamma = (price(spot + dSpot) - 2 * price(spot) + price(spot - dSpot)) /
    // (dSpot ^ 2)
    Eigen::MatrixXd gamma{(prices[Perturb::getIdx(Perturb::Idx::HiSpot)] -
                           (2.0 * prices[Perturb::getIdx(Perturb::Idx::Base)]) +
                           prices[Perturb::getIdx(Perturb::Idx::LoSpot)]) /
                          (hSpot * hSpot)};

    return GreeksResult{std::move(prices[Perturb::getIdx(Perturb::Idx::Base)]),
                        std::move(delta),
                        std::move(gamma),
                        std::move(vega),
                        std::move(theta),
                        std::move(rho)};
  }
};
