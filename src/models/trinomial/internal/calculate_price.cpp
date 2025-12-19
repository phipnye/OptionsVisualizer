#include "OptionsVisualizer/models/trinomial/internal/calculate_price.hpp"
#include "OptionsVisualizer/core/Enums.hpp"
#include "OptionsVisualizer/models/trinomial/internal/constants.hpp"
#include "OptionsVisualizer/models/trinomial/internal/helpers.hpp"
#include <Eigen/Dense>
#include <cassert>
#include <cmath>
#include <vector>

namespace models::trinomial {

Eigen::MatrixXd calculatePrice(Eigen::DenseIndex nSigma, Eigen::DenseIndex nStrike, double spot, double r, double q,
                               const Eigen::MatrixXd& strikesGrid, const Eigen::MatrixXd& sigmasGrid, double tau,
                               Enums::OptionType optType) {
    // --- Setup

    // Discrete time steps
    const double dTau{tau / constants::trinomialDepth};

    // Calculate u based on the first column of sigmasGrid
    // Stock price multipliers: u = e^(sigma * sqrt(3dt)); d = 1 / u
    const Eigen::VectorXd u{(sigmasGrid.col(0).array() * std::sqrt(3.0 * dTau)).exp()};

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
    const Eigen::MatrixXd driftFactor{scalingTerm * logStockDrift};

    // --- Risk-neutral probabilities

    // Probability of upward price movement: p_u = sqrt(dt / 12 * sigma^2) * (r - q * sigma^2 / 2) + 1 / 6
    const Eigen::MatrixXd pU{driftFactor.array() + (1.0 / 6.0)};

    // Probability of downward price movement: p_d = -sqrt(dt / 12 * sigma^2) * (r - q * sigma^2 / 2) + 1 / 6
    const Eigen::MatrixXd pD{(-driftFactor.array()) + (1.0 / 6.0)};

    // p_m = 1 - p_u - p_d approx= 2/3
    const Eigen::MatrixXd pM{1.0 - pU.array() - pD.array()};

    // --- Compute price using backward induction

    using constants::trinomialDepth;

    // Calculate option values at expiration
    const Eigen::MatrixXd expirationSpot{
        helpers::buildSpotLattice(spot, u, trinomialDepth, nSigma)}; // shape: [node, sigma]

    // Calculate payoff at expiration (intrinsic value only at expriation)
    std::vector<Eigen::MatrixXd> optionValues{helpers::intrinsicValue(
        expirationSpot, strikesGrid, 2 * trinomialDepth + 1, nSigma, nStrike, optType)}; // shape: [node, sigma, strike]

    // Backward induction
    for (Eigen::DenseIndex depth{trinomialDepth - 1}; depth > -1; --depth) { // Eigen::DenseIndex (long int) is signed
        // Compute spot prices at this timestep
        const Eigen::DenseIndex nNodes{2 * depth + 1};
        const Eigen::MatrixXd spotsDepth{helpers::buildSpotLattice(spot, u, depth, nSigma)};

        // intrinsicValue returns a new vector of matrices for the exercise boundary at this depth
        const std::vector<Eigen::MatrixXd> exerciseValue{
            helpers::intrinsicValue(spotsDepth, strikesGrid, nNodes, nSigma, nStrike, optType)};

        // Reuse the vector for the new time step
        std::vector<Eigen::MatrixXd> nextOptionValues(nNodes, Eigen::MatrixXd(nSigma, nStrike));

        for (Eigen::DenseIndex node{0}; node < nNodes; ++node) {
            // Trinomial logic: node i at current depends on nodes (i + 2, i + 1, i) (up, mid, down) from next depth
            const Eigen::MatrixXd& valUp{optionValues[node + 2]};
            const Eigen::MatrixXd& valMid{optionValues[node + 1]};
            const Eigen::MatrixXd& valDown{optionValues[node]};

            // Calculate expected value: (pU*Up + pM*Mid + pD*Down) * discount
            // Since pU, pM, pD are [nSigma x nStrike], use element-wise array multiplication
            const Eigen::MatrixXd continuationValue{
                (pU.array() * valUp.array() + pM.array() * valMid.array() + pD.array() * valDown.array()) *
                discountFactor};

            // Update optionValues: American early exercise check
            nextOptionValues[node] = continuationValue.cwiseMax(exerciseValue[node]);
        }

        optionValues = std::move(nextOptionValues);
    }

    // The vector at the end of the loop has size 1 (the root node)
    assert(optionValues.size() == 1);
    return optionValues[0];
}

} // namespace models::trinomial