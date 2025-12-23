#pragma once

#include "OptionsVisualizer/core/Enums.hpp"
#include "OptionsVisualizer/models/trinomial/internal/constants.hpp"
#include "OptionsVisualizer/models/trinomial/internal/helpers.hpp"
#include <Eigen/Dense>
#include <cassert>
#include <cmath>
#include <utility>
#include <vector>

namespace models::trinomial {
    template<Enums::OptionType T>
    Eigen::MatrixXd calculatePrice(const Eigen::DenseIndex nSigma, const Eigen::DenseIndex nStrike, const double spot,
                                   const double r, const double q,
                                   const auto &sigmasGrid, const Eigen::MatrixXd &strikesGrid,
                                   const double tau) {
        static_assert(T == Enums::OptionType::AmerCall || T == Enums::OptionType::AmerPut,
                      "Trinomial price evaluation only expected for American options");

        // --- Setup

        // Discrete time steps
        const double dTau{tau / constants::trinomialDepth};

        // Calculate u based on the first column of sigmasGrid
        // Stock price multipliers: u = e^(sigma * sqrt(3dt)); d = 1 / u
        const Eigen::VectorXd u{(sigmasGrid.matrix().col(0).array() * std::sqrt(3.0 * dTau)).exp().eval()};

        // Single-step discount factor: discountFactor = e^(-r * dt)
        const double discountFactor{std::exp(-r * dTau)};

        // --- Intermediate risk-neutral probability terms (see Hull - Ch.20 (444))

        // Use auto for expression templates to avoid temporary matrix allocations for intermediate steps
        const auto sigmasSq{sigmasGrid.array().square()};

        // Drift factor scaling term: sqrt(dt / 12 * sigma^2)
        const auto scalingTerm{(dTau / (12.0 * sigmasSq)).sqrt()};

        // Log stock drift: r - q * sigma^2 / 2
        const auto logStockDrift{(r - q) - (sigmasSq / 2.0)};

        // Risk-neutral drift factor
        const Eigen::MatrixXd driftFactor{(scalingTerm * logStockDrift).eval()};

        // --- Risk-neutral probabilities

        // Probability of upward price movement: p_u = sqrt(dt / 12 * sigma^2) * (r - q * sigma^2 / 2) + 1 / 6
        const Eigen::MatrixXd pU{driftFactor.array() + (1.0 / 6.0)};

        // Probability of downward price movement: p_d = -sqrt(dt / 12 * sigma^2) * (r - q * sigma^2 / 2) + 1 / 6
        const Eigen::MatrixXd pD{(-driftFactor.array()) + (1.0 / 6.0)};

        // p_m = 1 - p_u - p_d approx= 2/3
        const Eigen::MatrixXd pM{1.0 - pU.array() - pD.array()};

        // --- Compute price using backward induction

        using constants::trinomialDepth;

        // Pre-allocate buffers and exercise storage (avoids heap allocations in the loop)
        constexpr Eigen::DenseIndex maxNodes{2 * trinomialDepth + 1};
        std::vector<Eigen::MatrixXd> nextOptionValues(maxNodes, Eigen::MatrixXd(nSigma, nStrike));
        std::vector<Eigen::MatrixXd> currOptionValues(maxNodes, Eigen::MatrixXd(nSigma, nStrike));

        // Calculate option values at expiration
        const Eigen::MatrixXd expirationSpot{
            helpers::buildSpotLattice(spot, u, trinomialDepth, nSigma)
        }; // shape: [node, sigma]

        // Calculate payoff at expiration (intrinsic value only at expriation)
        for (Eigen::DenseIndex node{0}; node < maxNodes; ++node) {
            nextOptionValues[node] = helpers::intrinsicValue<T>(strikesGrid, expirationSpot.row(node).transpose());
        }

        // Backward induction
        for (Eigen::DenseIndex depth{trinomialDepth - 1}; depth > -1; --depth) {
            // Eigen::DenseIndex (long int) is signed
            // Compute spot prices at this timestep
            const Eigen::MatrixXd spotsDepth{helpers::buildSpotLattice(spot, u, depth, nSigma)};
            const Eigen::DenseIndex nNodes{2 * depth + 1};

            for (Eigen::DenseIndex node{0}; node < nNodes; ++node) {
                // Node i at current depends on nodes (i + 2, i + 1, i) (up, mid, down) from next depth
                const Eigen::MatrixXd &valUp{nextOptionValues[static_cast<std::size_t>(node + 2)]};
                const Eigen::MatrixXd &valMid{nextOptionValues[static_cast<std::size_t>(node + 1)]};
                const Eigen::MatrixXd &valDown{nextOptionValues[static_cast<std::size_t>(node)]};

                // Calculate expected value: (pU * valUp + pM * valMid + pD * valDown) * discount
                const Eigen::MatrixXd continuationValue{
                    (pU.array() * valUp.array() + pM.array() * valMid.array() + pD.array() * valDown.array()) *
                    discountFactor
                };

                // Update optionValues: American early exercise check
                currOptionValues[node] =
                        continuationValue.cwiseMax(
                            helpers::intrinsicValue<T>(strikesGrid, spotsDepth.row(node).transpose()));
            }

            // Swap buffers
            std::swap(nextOptionValues, currOptionValues);
        }

        return std::move(nextOptionValues[0]); // root node value at index 0
    }
} // namespace models::trinomial
