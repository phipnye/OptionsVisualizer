#pragma once

#include <BS_thread_pool.hpp>
#include <Eigen/Dense>
#include <array>
#include <cstddef>
#include <cstdint>
#include <future>
#include <utility>

#include "OptionsVisualizer/core/Enums.hpp"
#include "OptionsVisualizer/core/arrayUtils.hpp"
#include "OptionsVisualizer/core/globals.hpp"
#include "OptionsVisualizer/models/trinomial/internal/calculate_price.hpp"
#include "OptionsVisualizer/pricing/GreeksResult.hpp"

class PricingSurface {
  using GridArray = std::array<Eigen::ArrayXXd, globals::nGrids>;

  // --- Data-members
  const Eigen::ArrayXXd sigmasGrid_;
  const Eigen::ArrayXXd strikesGrid_;
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
    double dR_{0.0};         // perturbation in risk-free rate (rho)
    double sigmaMult_{1.0};  // perturbation multiplier in volatility (sigma)
  };

  // Enum for mapping perturbations
  enum class PerturbKind : std::uint8_t {
    Base,
    LoSpot,
    HiSpot,
    LoSigma,
    HiSigma,
    LoTau,
    HiTau,
    LoRho,
    HiRho,
    COUNT
  };

  [[nodiscard]] static constexpr std::size_t idx(const PerturbKind k) noexcept {
    return static_cast<std::size_t>(k);
  }

 public:
  explicit PricingSurface(Eigen::Index nSigma, Eigen::Index nStrike,
                          double spot, double r, double q, double sigmaLo,
                          double sigmaHi, double strikeLo, double strikeHi,
                          double tau, BS::thread_pool<>& pool);

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

    // Relative epsilons to estimate derivatives using finite differences (a
    // larger relative percentage is used for the spot price since we use it for
    // second order derivatives and small perturbations lead to "jagged" results
    // across the grid of strike x sigma)
    const double dSpot{spot_ * 0.05};
    const double dTau{tau_ * 0.01};
    const double dR{r_ * 0.01};

    // Use a relative percentage shift for the grid of sigmas
    static constexpr double sigmaShift{0.01};

    // --- Compute prices

    // List of perturbations to compute finite-difference approximations
    static constexpr std::size_t nPerturbs{idx(PerturbKind::COUNT)};
    using PerturbTable = std::array<std::pair<PerturbKind, Perturb>, nPerturbs>;
    const PerturbTable perturbs{{
        // Base price (no perturbation)
        {PerturbKind::Base, Perturb{}},

        // Spot perturbations for delta and gamma
        {PerturbKind::LoSpot, Perturb{.dSpot_ = -dSpot}},
        {PerturbKind::HiSpot, Perturb{.dSpot_ = dSpot}},

        // Sigma perturbations for vega
        {PerturbKind::LoSigma, Perturb{.sigmaMult_ = 1.0 - sigmaShift}},
        {PerturbKind::HiSigma, Perturb{.sigmaMult_ = 1.0 + sigmaShift}},

        // Tau perturbations for theta
        {PerturbKind::LoTau, Perturb{.dTau_ = -dTau}},
        {PerturbKind::HiTau, Perturb{.dTau_ = dTau}},

        // Risk-free rate perturbations for rho
        {PerturbKind::LoRho, Perturb{.dR_ = -dR}},
        {PerturbKind::HiRho, Perturb{.dR_ = dR}},
    }};

    // Prepare an array to hold futures for asynchronous price calculations
    std::array<std::future<Eigen::ArrayXXd>, nPerturbs> futures{};

    // Launch asynchronous tasks for each perturbation using the thread pool
    for (std::size_t idx{0}; idx < nPerturbs; ++idx) {
      const auto& [_, p]{perturbs[idx]};

      futures[idx] = pool_.submit_task([p, this] {
        // Compute the option price with the specified perturbation applied
        return models::trinomial::calculatePrice<OptType>(
            this->spot_ + p.dSpot_,  // perturbed spot
            this->r_ + p.dR_,        // perturbed risk-free rate
            this->q_,
            this->sigmasGrid_ * p.sigmaMult_,  // perturbed sigmas
            this->strikesGrid_,
            this->tau_ + p.dTau_);  // perturbed time to maturity
      });
    }

    // Retrieve the computed prices from the futures
    std::array prices{Utils::preallocArrays<nPerturbs>(sigmasGrid_.rows(),
                                                       sigmasGrid_.cols())};

    for (std::size_t idx{0}; idx < nPerturbs; ++idx) {
      prices[idx] = futures[idx].get();
    }

    const auto& base{prices[idx(PerturbKind::Base)]};
    const auto& loSpot{prices[idx(PerturbKind::LoSpot)]};
    const auto& hiSpot{prices[idx(PerturbKind::HiSpot)]};
    const auto& loSigma{prices[idx(PerturbKind::LoSigma)]};
    const auto& hiSigma{prices[idx(PerturbKind::HiSigma)]};
    const auto& loTau{prices[idx(PerturbKind::LoTau)]};
    const auto& hiTau{prices[idx(PerturbKind::HiTau)]};
    const auto& loRho{prices[idx(PerturbKind::LoRho)]};
    const auto& hiRho{prices[idx(PerturbKind::HiRho)]};

    // --- First-order derivatives (delta, vega, theta, rho)

    // Compute delta (first derivative w.r.t spot)
    Eigen::ArrayXXd delta{firstOrderCdm(loSpot, hiSpot, dSpot)};

    // Compute vega (first derivative w.r.t sigma)
    const auto dSigma{sigmasGrid_ * sigmaShift};
    Eigen::ArrayXXd vega{firstOrderCdm(loSigma, hiSigma, dSigma)};

    // Compute theta (negative first derivative w.r.t tau)
    Eigen::ArrayXXd theta{-firstOrderCdm(loTau, hiTau, dTau)};

    // Compute rho (first derivative w.r.t r)
    Eigen::ArrayXXd rho{firstOrderCdm(loRho, hiRho, dR)};

    // --- Second-order derivatives (gamma)

    // Compute gamma (second derivative w.r.t spot)
    Eigen::ArrayXXd gamma{secondOrderCdm(loSpot, base, hiSpot, dSpot)};

    return GreeksResult{std::move(prices[idx(PerturbKind::Base)]),
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
