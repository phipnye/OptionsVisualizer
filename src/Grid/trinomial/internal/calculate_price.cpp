#include "OptionsVisualizer/models/trinomial/calculate_price.hpp"
#include "OptionsVisualizer/Grid/Enums.hpp"
#include "OptionsVisualizer/models/trinomial/constants.hpp"
#include "OptionsVisualizer/models/trinomial/helpers.hpp"
#include <Eigen/Dense>
#include <unsupported/Eigen/CXX11/Tensor>

namespace models::trinomial {

Eigen::Tensor<double, 2> calculatePrice(double spot, const Eigen::Tensor<double, 2>& strikesGrid, double r, double q,
                                        const Eigen::Tensor<double, 2>& sigmasGrid, double tau,
                                        Eigen::DenseIndex nSigma, Eigen::DenseIndex nStrike, OptionType optType) {
    // --- Setup

    // Discrete time steps
    const double dTau{tau / constants::trinomialDepth};

    // Calculate u based on the first column of sigmasGrid
    // Stock price multipliers: u = e^(sigma * sqrt(3dt)); d = 1 / u
    const Eigen::Tensor<double, 1> u{(sigmasGrid.chip(0, 1) * std::sqrt(3.0 * dTau)).exp()};

    // Single-step discount factor: discountFactor = e^(-r * dt)
    const double discountFactor{std::exp(-r * dTau)};

    // --- Intermediate risk-neutral probability terms (see Hull - Ch.20 (444))

    // Drift factor scaling term: sqrt(dt / 12 * sigma^2)
    const Eigen::Tensor<double, 2> scalingTerm{(dTau / (12.0 * sigmasGrid.square())).sqrt()};

    // Log stock drift: r - q * sigma^2 / 2
    const Eigen::Tensor<double, 2> logStockDrift{(r - q) - (sigmasGrid.square() / 2.0)};

    // Risk-neutral drift factor
    const Eigen::Tensor<double, 2> driftFactor{scalingTerm * logStockDrift};

    // --- Risk-neutral probabilities

    // Probability of upward price movement: p_u = sqrt(dt / 12 * sigma^2) * (r - q * sigma^2 / 2) + 1 / 6
    const Eigen::Tensor<double, 3> pU{
        (driftFactor + (1.0 / 6.0)).reshape(Eigen::array<Eigen::DenseIndex, 3>{1, nSigma, nStrike})};

    // Probability of downward price movement: p_d = -sqrt(dt / 12 * sigma^2) * (r - q * sigma^2 / 2) + 1 / 6
    const Eigen::Tensor<double, 3> pD{
        (-driftFactor + (1.0 / 6.0)).reshape(Eigen::array<Eigen::DenseIndex, 3>{1, nSigma, nStrike})};

    // p_m = 1 - p_u - p_d approx= 2/3
    const Eigen::Tensor<double, 3> pM{1.0 - pU - pD};

    // --- Compute price using backward induction

    using constants::trinomialDepth;

    // Calculate option values at expiration
    const Eigen::Tensor<double, 2> expirationSpot{
        helpers::buildSpotLattice(spot, u, trinomialDepth, nSigma)}; // shape: [node, sigma]

    // Calculate payoff at expiration (intrinsic value only at expriation)
    Eigen::Tensor<double, 3> optionValues{helpers::intrinsicValue(
        expirationSpot, strikesGrid, 2 * trinomialDepth + 1, nSigma, nStrike, optType)}; // shape: [node, sigma, strike]

    // Backward induction
    for (Eigen::DenseIndex depth{trinomialDepth - 1}; depth > -1; --depth) { // Eigen::DenseIndex (long int) is signed
        // Compute spot prices at this timestep
        const Eigen::DenseIndex nNodes{2 * depth + 1};
        const Eigen::Tensor<double, 2> spotsDepth{helpers::buildSpotLattice(spot, u, depth, nSigma)};

        // Compute expected value using risk-neutral probabilities
        static const Eigen::array<Eigen::DenseIndex, 3> offsetUp{2, 0, 0};
        static const Eigen::array<Eigen::DenseIndex, 3> offsetMid{1, 0, 0};
        static const Eigen::array<Eigen::DenseIndex, 3> offsetDown{0, 0, 0};

        // Broadcast probabilities across the nNodes and nStrike dimensions
        const Eigen::array<Eigen::DenseIndex, 3> shape{nNodes, nSigma, nStrike};
        const Eigen::array<Eigen::DenseIndex, 3> probBcast{nNodes, 1, 1};
        const Eigen::Tensor<double, 3> expectedValue{pU.broadcast(probBcast) * optionValues.slice(offsetUp, shape) +
                                                     pM.broadcast(probBcast) * optionValues.slice(offsetMid, shape) +
                                                     pD.broadcast(probBcast) * optionValues.slice(offsetDown, shape)};

        // Discount back one timestep
        const Eigen::Tensor<double, 3> continuationValue{discountFactor * expectedValue};

        // Compute intrinsic value
        const Eigen::Tensor<double, 3> exerciseValue{
            helpers::intrinsicValue(spotsDepth, strikesGrid, nNodes, nSigma, nStrike, optType)};

        // Update optionValues at this timestep
        optionValues = continuationValue.cwiseMax(exerciseValue);
    }

    // remove the singleton node dimension
    return optionValues.chip(0, 0);
}

} // namespace models::trinomial