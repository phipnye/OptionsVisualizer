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
  using GridArray = std::array<Eigen::ArrayXXd, globals::nGrids>;

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
    double dSpot_{0.0};      // perturbation in spot price
    double dTau_{0.0};       // perturbation in time to expiry (tau)
    double dRho_{0.0};       // perturbation in risk-free rate (rho)
    double sigmaMult_{1.0};  // perturbation multiplier in volatility (sigma)
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

    [[nodiscard]] static constexpr std::size_t idx(Idx i) {
      return static_cast<std::size_t>(i);
    }
  };

 public:
  PricingSurface(Eigen::Index nSigma, Eigen::Index nStrike, double spot,
                 double r, double q, double sigmaLo, double sigmaHi,
                 double strikeLo, double strikeHi, double tau,
                 BS::thread_pool<>& pool);

  // Helper to append greek results together
  static void appendGreeks(GridArray& grids, Enums::OptionType optType,
                           GreeksResult&& g);

  // Compute grids for all greeks and option types
  [[nodiscard]] GridArray calculateGrids() const;

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

    // Relative epsilons to estimate derviates using finite differences (a
    // larger relative percentage is used for the spot price since we use it for
    // second order derivatives and small perturbations lead to "jagged" results
    // across the grid of strike x sigma)
    const double dSpot{spot_ * 0.05};
    const double dTau{tau_ * 0.01};
    const double dRho{r_ * 0.01};

    // Use a relative percentage shift for the grid of sigmas
    constexpr double sigmaShift{0.01};

    // --- Compute prices

    // List of perturbations to compute finite-difference approximations
    const std::array perturbations{
        // Base price (no perturbation)
        Perturb{.idx_ = Perturb::Idx::Base},

        // Spot perturbations for delta and gamma
        Perturb{.dSpot_ = -dSpot, .idx_ = Perturb::Idx::LoSpot},
        Perturb{.dSpot_ = dSpot, .idx_ = Perturb::Idx::HiSpot},

        // Sigma perturbations for vega
        Perturb{.sigmaMult_ = 1.0 - sigmaShift, .idx_ = Perturb::Idx::LoSigma},
        Perturb{.sigmaMult_ = 1.0 + sigmaShift, .idx_ = Perturb::Idx::HiSigma},

        // Tau perturbations for theta
        Perturb{.dTau_ = -dTau, .idx_ = Perturb::Idx::LoTau},
        Perturb{.dTau_ = dTau, .idx_ = Perturb::Idx::HiTau},

        // Risk-free rate perturbations for rho
        Perturb{.dRho_ = -dRho, .idx_ = Perturb::Idx::LoRho},
        Perturb{.dRho_ = dRho, .idx_ = Perturb::Idx::HiRho}};

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
            this->sigmasGrid_ * p.sigmaMult_,  // perturbed sigmas
            this->strikesGrid_,
            this->tau_ + p.dTau_);  // perturbed time
      }));
    }

    // Retrieve the computed prices from the futures
    std::vector<Eigen::ArrayXXd> prices(Perturb::idx(Perturb::Idx::Count));

    for (std::size_t pIdx{0}; pIdx < perturbations.size(); ++pIdx) {
      prices[Perturb::idx(perturbations[pIdx].idx_)] = futures[pIdx].get();
    }

    // --- First-order derivatives (delta, vega, theta, rho)

    // Compute delta (first derivative w.r.t spot) using CDM
    // delta = (price(spot + dSpot) - price(spot - dSpot)) / (2 * dSpot)
    Eigen::ArrayXXd delta{
        firstOrderCdm(prices[Perturb::idx(Perturb::Idx::LoSpot)],
                      prices[Perturb::idx(Perturb::Idx::HiSpot)], dSpot)};

    // Compute vega (first derivative w.r.t sigma) using CDM
    // vega = (price(sigma + dSigma) - price(sigma - dSigma)) / (2 * dSigma)
    const auto hSigma{sigmasGrid_ * sigmaShift};
    Eigen::ArrayXXd vega{
        firstOrderCdm(prices[Perturb::idx(Perturb::Idx::LoSigma)],
                      prices[Perturb::idx(Perturb::Idx::HiSigma)], hSigma)};

    // Compute theta (negative first derivative w.r.t tau) using CDM
    // theta = -(price(tau + dTau) - price(tau - dTau)) / (2 * dTau)
    Eigen::ArrayXXd theta{
        -firstOrderCdm(prices[Perturb::idx(Perturb::Idx::LoTau)],
                       prices[Perturb::idx(Perturb::Idx::HiTau)], dTau)};

    // Compute rho (first derivate w.r.t r) using CDM
    // rho = (price(r + dR) - price(r - dR)) / (2 * dR)
    Eigen::ArrayXXd rho{firstOrderCdm(prices[Perturb::idx(Perturb::Idx::LoRho)],
                                      prices[Perturb::idx(Perturb::Idx::HiRho)],
                                      dRho)};

    // --- Second-order derivatives (gamma)

    // Compute gamma (second derivative w.r.t spot) using CDM
    // gamma = (price(spot + dSpot) - 2 * price(spot) + price(spot - dSpot)) /
    // (dSpot ^ 2)
    Eigen::ArrayXXd gamma{
        secondOrderCdm(prices[Perturb::idx(Perturb::Idx::LoSpot)],
                       prices[Perturb::idx(Perturb::Idx::Base)],
                       prices[Perturb::idx(Perturb::Idx::HiSpot)], dSpot)};

    return GreeksResult{std::move(prices[Perturb::idx(Perturb::Idx::Base)]),
                        std::move(delta),
                        std::move(gamma),
                        std::move(vega),
                        std::move(theta),
                        std::move(rho)};
  }

  // --- Helpers to compute first and second order derivatives using central
  // difference method (templated to handle array or scalar epsilons)

  template <typename T>
  static Eigen::ArrayXXd firstOrderCdm(const Eigen::ArrayXXd& lo,
                                       const Eigen::ArrayXXd& hi,
                                       const T& eps) {
    return (hi - lo) / (2.0 * eps);
  }

  template <typename T>
  static Eigen::ArrayXXd secondOrderCdm(const Eigen::ArrayXXd& lo,
                                        const Eigen::ArrayXXd& base,
                                        const Eigen::ArrayXXd& hi,
                                        const T& eps) {
    return (hi - (2.0 * base) + lo) / (eps * eps);
  }
};
