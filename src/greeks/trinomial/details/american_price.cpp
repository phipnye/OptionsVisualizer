#include "OptionsVisualizer/greeks/trinomial/details/american_price.hpp"
#include "OptionsVisualizer/greeks/trinomial/details/constants.hpp"
#include "OptionsVisualizer/greeks/trinomial/details/helpers.hpp"
#include <Eigen/Dense>
#include <unsupported/Eigen/CXX11/Tensor>

namespace greeks::trinomial::details {

Eigen::Tensor<double, 2> americanPrice(double spot, const Eigen::Tensor<double, 2>& strikesGrid, double r, double q,
                                       const Eigen::Tensor<double, 2>& sigmasGrid, double tau, grid::index::Dims dims,
                                       bool isCall) {
    // Get setup terms
    const auto [u, discountFactor, pU, pM, pD]{helpers::setupTrinomial(r, q, sigmasGrid, tau, dims)};

    // --- Compute price using backward induction

    // Calculate option values at expiration
    const Eigen::Tensor<double, 2> expirationSpot{
        helpers::buildSpotLattice(spot, u, constants::trinomialDepth, dims)}; // shape: [node, sigma]

    // Calculate payoff at expiration (intrinsic value only at expriation)
    Eigen::Tensor<double, 3> optionValues{
        helpers::intrinsicValue(expirationSpot, strikesGrid, dims, isCall)}; // shape: [node, sigma, strike]

    // Backward induction
    for (Eigen::DenseIndex depth{constants::trinomialDepth - 1}; depth > -1; --depth) { // Eigen::DenseIndex is signed
        // Compute spot prices at this timestep
        const Eigen::DenseIndex nNodes{2 * depth + 1};
        const Eigen::Tensor<double, 2> spotsDepth{helpers::buildSpotLattice(spot, u, depth, dims)};

        // Compute expected value using risk-neutral probabilities
        static const Eigen::array<Eigen::DenseIndex, 3> offsetUp{2, 0, 0};
        static const Eigen::array<Eigen::DenseIndex, 3> offsetMid{1, 0, 0};
        static const Eigen::array<Eigen::DenseIndex, 3> offsetDown{0, 0, 0};

        // Shape [nNodes, nSigma, nStrike]
        const Eigen::array<Eigen::DenseIndex, 3> shape{nNodes, optionValues.dimension(1), optionValues.dimension(2)};

        // Broadcast probabilities across the nNodes and nStrike dimensions
        const Eigen::array<Eigen::DenseIndex, 3> probBcast{nNodes, 1, 1};
        const Eigen::Tensor<double, 3> expectedValue{pU.broadcast(probBcast) * optionValues.slice(offsetUp, shape) +
                                                     pM.broadcast(probBcast) * optionValues.slice(offsetMid, shape) +
                                                     pD.broadcast(probBcast) * optionValues.slice(offsetDown, shape)};

        // Discount back one timestep
        const Eigen::Tensor<double, 3> continuationValue{discountFactor * expectedValue};

        // Compute intrinsic value
        const Eigen::Tensor<double, 3> exerciseValue{helpers::intrinsicValue(spotsDepth, strikesGrid, dims, isCall)};

        // Update optionValues at this timestep
        optionValues = continuationValue.cwiseMax(exerciseValue);
    }

    // remove the singleton node dimension
    return optionValues.chip(0, 0);
}

} // namespace greeks::trinomial::details