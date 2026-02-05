#pragma once

#include <Eigen/Dense>
#include <cmath>
#include <type_traits>
#include <utility>
#include <vector>

#include "OptionsVisualizer/core/Enums.hpp"
#include "OptionsVisualizer/models/trinomial/internal/constants.hpp"
#include "OptionsVisualizer/models/trinomial/internal/helpers.hpp"

namespace models::trinomial {

template <Enums::OptionType OptType>
[[nodiscard]] Eigen::ArrayXXd calculatePrice(
    const Eigen::Index nSigma, const Eigen::Index nStrike, const double spot,
    const double r, const double q, const Eigen::ArrayXXd& sigmasGrid,
    const Eigen::ArrayXXd& strikesGrid, const double tau) {
  // Make sure we only use this for American option pricing
  static_assert(
      OptType == Enums::OptionType::AmerCall ||
          OptType == Enums::OptionType::AmerPut,
      "Trinomial price evaluation only expected for American options");

  // --- Setup

  // Discrete time steps
  const double dTau{tau / static_cast<double>(constants::trinomialDepth)};

  // Stock price multipliers: u = e^(sigma * sqrt(3dt)); d = 1 / u
  const Eigen::ArrayXd u{(sigmasGrid.col(0) * std::sqrt(3.0 * dTau)).exp()};

  // Single-step discount factor: discountFactor = e^(-r * dt)
  const double discountFactor{std::exp(-r * dTau)};

  // --- Intermediate risk-neutral probability terms (see Hull - Ch.20 (444))

  // Drift factor scaling term: sqrt(dt / 12 * sigma^2)
  const Eigen::ArrayXXd sigmasSq{sigmasGrid.square()};
  const auto scalingTerm{(dTau / (12.0 * sigmasSq)).sqrt()};

  // Log stock drift: r - q - sigma^2 / 2
  const auto logStockDrift{(r - q) - (0.5 * sigmasSq)};

  // Risk-neutral drift factor
  const auto driftFactor{scalingTerm * logStockDrift};

  // --- Risk-neutral probabilities

  // Probability of upward price movement: p_u = sqrt(dt / 12 * sigma^2) * (r -
  // q * sigma^2 / 2) + 1 / 6
  const Eigen::ArrayXXd pU{driftFactor + (1.0 / 6.0)};

  // Probability of downward price movement: p_d = -sqrt(dt / 12 * sigma^2) * (r
  // - q * sigma^2 / 2) + 1 / 6 = 2 / 6 - p_u
  const Eigen::ArrayXXd pD{(2.0 / 6.0) - pU};

  // p_m = 1 - p_u - p_d approx= 2/3
  const Eigen::ArrayXXd pM{1.0 - pU - pD};

  // --- Compute price using backward induction

  // Pre-allocate buffers
  using constants::trinomialDepth;
  constexpr Eigen::Index maxNodes{2 * trinomialDepth + 1};
  std::vector nextOptionValues(maxNodes, Eigen::ArrayXXd{nSigma, nStrike});
  std::vector currOptionValues(maxNodes, Eigen::ArrayXXd{nSigma, nStrike});

  // Calculate option values at expiration (shape: [node, sigma])
  const Eigen::ArrayXXd expirationSpot{
      helpers::buildSpotLattice(spot, u, trinomialDepth)};

  // Calculate payoff at expiration (intrinsic value only at expriation)
  for (Eigen::Index node{0}; node < maxNodes; ++node) {
    nextOptionValues[node] = helpers::intrinsicValue<OptType>(
        strikesGrid, expirationSpot.row(node).transpose());
  }

  // Backward induction
  for (Eigen::Index depth{trinomialDepth - 1}; depth > -1; --depth) {
    // Need depth to be signed for loop to behave properly
    static_assert(
        std::is_signed_v<decltype(depth)>,
        "Expected a signed type for depth in trinomial price calculation");

    // Compute spot prices at this timestep
    const Eigen::ArrayXXd spotsDepth{helpers::buildSpotLattice(spot, u, depth)};
    const Eigen::Index nNodes{2 * depth + 1};

    for (Eigen::Index node{0}; node < nNodes; ++node) {
      // Node i at current depends on nodes (i + 2, i + 1, i) (up, mid, down)
      // from next depth
      const std::size_t nodeT{static_cast<std::size_t>(node)};
      const Eigen::ArrayXXd& valU{nextOptionValues[nodeT + 2]};
      const Eigen::ArrayXXd& valM{nextOptionValues[nodeT + 1]};
      const Eigen::ArrayXXd& valD{nextOptionValues[nodeT]};

      // Calculate expected value: (pU * valUp + pM * valMid + pD * valDown) *
      // discount
      const auto continuationValue{(pU * valU + pM * valM + pD * valD) *
                                   discountFactor};

      // Update optionValues: American early exercise check
      currOptionValues[node] =
          continuationValue.cwiseMax(helpers::intrinsicValue<OptType>(
              strikesGrid, spotsDepth.row(node).transpose()));
    }

    // Swap buffers
    std::swap(nextOptionValues, currOptionValues);
  }

  return std::move(nextOptionValues[0]);  // root node value at index 0
}

}  // namespace models::trinomial
