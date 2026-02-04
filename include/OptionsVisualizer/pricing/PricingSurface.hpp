#pragma once

// ReSharper disable once CppUnusedIncludeDirective
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
  const Eigen::ArrayXXd sigmasGrid_;
  const Eigen::ArrayXXd strikesGrid_;
  const Eigen::Index nSigma_;
  const Eigen::Index nStrike_;
  const double spot_;
  const double r_;
  const double q_;
  const double tau_;
  BS::thread_pool<>& pool_;

  // Define a simple struct to hold small perturbations for spot, sigma, and
  // tau
  struct Perturb {
    double dSpot_{0.0};   // perturbation in spot price
    double dSigma_{0.0};  // perturbation in volatility (sigma)
    double dTau_{0.0};    // perturbation in time to expiry (tau)
    double dRho_{0.0};    // perturbation in risk-free rate (rho)
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
    } idx_;

    [[nodiscard]] static constexpr std::size_t getIdx(Idx idx) {
      return static_cast<std::size_t>(idx);
    }
  };

 public:
  PricingSurface(Eigen::Index nSigma, Eigen::Index nStrike, double spot,
                 double r, double q, double sigmaLo, double sigmaHi,
                 double strikeLo, double strikeHi, double tau,
                 BS::thread_pool<>& pool);

  // Compute grids for all greeks and option types
  [[nodiscard]] std::array<Eigen::ArrayXXd, globals::nGrids> calculateGrids()
      const;

 private:
  // --- Black-Scholes-Merton
  [[nodiscard]] GreeksResult bsmCallGreeks() const;

  [[nodiscard]] GreeksResult bsmPutGreeks(
      const GreeksResult& callResults) const;

  // --- Trinomial tree
  template <Enums::OptionType OptType>
  [[nodiscard]] GreeksResult trinomialGreeks() const {
    static_assert(
        OptType == Enums::OptionType::AmerCall ||
            OptType == Enums::OptionType::AmerPut,
        "Trinomial greeks calculation only expected for American options");

    // Epsilons to estimate derviates using finite differences
    const double hSpot{spot_ * 0.05};
    const double hTau{tau_ * 0.01};
    constexpr double hSigma{0.01};  // 1% shift for volatility
    constexpr double hRho{0.01};    // 100 basis points shift for risk-free rate

    // --- Compute prices

    // List of perturbations to compute finite-difference approximations
    const std::array perturbations{
        // Base price (no perturbation)
        Perturb{.idx_ = Perturb::Idx::Base},

        // Spot perturbations for delta and gamma
        Perturb{.dSpot_ = -hSpot, .idx_ = Perturb::Idx::LoSpot},
        Perturb{.dSpot_ = hSpot, .idx_ = Perturb::Idx::HiSpot},

        // Sigma perturbations for vega
        Perturb{.dSigma_ = -hSigma, .idx_ = Perturb::Idx::LoSigma},
        Perturb{.dSigma_ = hSigma, .idx_ = Perturb::Idx::HiSigma},

        // Tau perturbations for theta
        Perturb{.dTau_ = -hTau, .idx_ = Perturb::Idx::LoTau},
        Perturb{.dTau_ = hTau, .idx_ = Perturb::Idx::HiTau},

        // Risk-free rate perturbations for rho
        Perturb{.dRho_ = -hRho, .idx_ = Perturb::Idx::LoRho},
        Perturb{.dRho_ = hRho, .idx_ = Perturb::Idx::HiRho}};

    // Prepare a vector to hold futures for asynchronous price calculations
    std::vector<std::future<Eigen::ArrayXXd>> futures{};
    futures.reserve(perturbations.size());

    // Launch asynchronous tasks for each perturbation using the thread pool
    for (const Perturb& p : perturbations) {
      futures.emplace_back(pool_.submit_task([p, this] {
        // Compute the option price with the specified perturbation applied
        return models::trinomial::calculatePrice<OptType>(
            this->nSigma_, this->nStrike_,
            this->spot_ + p.dSpot_,  // perturbed spot
            this->r_ + p.dRho_,      // perturbed risk-free rate
            this->q_,
            (this->sigmasGrid_ + p.dSigma_),  // perturbed sigmas
            this->strikesGrid_,
            this->tau_ + p.dTau_);  // perturbed time
      }));
    }

    // Retrieve the computed prices from the futures
    std::vector<Eigen::ArrayXXd> prices(Perturb::getIdx(Perturb::Idx::Count));

    for (std::size_t pIdx{0}; pIdx < perturbations.size(); ++pIdx) {
      prices[Perturb::getIdx(perturbations[pIdx].idx_)] = futures[pIdx].get();
    }

    // --- First-order derivatives (delta, vega, theta, rho)

    const auto firstOrderCDM{[](const Eigen::ArrayXXd& lo,
                                const Eigen::ArrayXXd& hi,
                                const double eps) -> Eigen::ArrayXXd {
      return (hi - lo) / (2.0 * eps);
    }};

    // Compute delta (first derivative w.r.t spot) using CDM
    // delta = (price(spot + dSpot) - price(spot - dSpot)) / (2 * dSpot)
    Eigen::ArrayXXd delta{
        firstOrderCDM(prices[Perturb::getIdx(Perturb::Idx::LoSpot)],
                      prices[Perturb::getIdx(Perturb::Idx::HiSpot)], hSpot)};

    // Compute vega (first derivative w.r.t sigma) using CDM
    // vega = (price(sigma + dSigma) - price(sigma - dSigma)) / (2 * dSigma)
    Eigen::ArrayXXd vega{
        firstOrderCDM(prices[Perturb::getIdx(Perturb::Idx::LoSigma)],
                      prices[Perturb::getIdx(Perturb::Idx::HiSigma)], hSigma)};

    // Compute theta (negative first derivative w.r.t tau) using CDM
    // theta = -(price(tau + dTau) - price(tau - dTau)) / (2 * dTau)
    Eigen::ArrayXXd theta{
        -firstOrderCDM(prices[Perturb::getIdx(Perturb::Idx::LoTau)],
                       prices[Perturb::getIdx(Perturb::Idx::HiTau)], hTau)};

    // Compute rho (first derivate w.r.t r) using CDM
    Eigen::ArrayXXd rho{
        firstOrderCDM(prices[Perturb::getIdx(Perturb::Idx::LoRho)],
                      prices[Perturb::getIdx(Perturb::Idx::HiRho)], hRho)};

    // --- Second-order derivatives (gamma)

    const auto secondOrderCDM{
        [](const Eigen::ArrayXXd& lo, const Eigen::ArrayXXd& base,
           const Eigen::ArrayXXd& hi, const double eps) -> Eigen::ArrayXXd {
          return (hi - (2.0 * base) + lo) / (eps * eps);
        }};

    // Compute gamma (second derivative w.r.t spot) using CDM
    // gamma = (price(spot + dSpot) - 2 * price(spot) + price(spot - dSpot)) /
    // (dSpot ^ 2)
    Eigen::ArrayXXd gamma{
        secondOrderCDM(prices[Perturb::getIdx(Perturb::Idx::LoSpot)],
                       prices[Perturb::getIdx(Perturb::Idx::Base)],
                       prices[Perturb::getIdx(Perturb::Idx::HiSpot)], hSpot)};

    return GreeksResult{std::move(prices[Perturb::getIdx(Perturb::Idx::Base)]),
                        std::move(delta),
                        std::move(gamma),
                        std::move(vega),
                        std::move(theta),
                        std::move(rho)};
  }
};
