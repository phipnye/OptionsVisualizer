#include "OptionsVisualizer/greeks/trinomial/details/american_price.hpp"
#include "OptionsVisualizer/greeks/trinomial/details/constants.hpp"
#include "OptionsVisualizer/greeks/trinomial/details/helpers.hpp"
#include <torch/torch.h>

namespace greeks::trinomial::details {

torch::Tensor americanPrice(const torch::Tensor& spot, const torch::Tensor& strikes, const torch::Tensor& r,
                            const torch::Tensor& q, const torch::Tensor& sigmas, const torch::Tensor& tau,
                            bool isCall) {
    // Get setup terms
    const auto [dTau, u, d, discountFactor, pU, pM, pD]{helpers::setupTrinomial(r, q, sigmas, tau)};

    // --- Compute price using backward induction

    // Calculate option values at expiration
    const std::int64_t numNodes{2 * constants::trinomialDepth + 1};
    const torch::Tensor expirationSpot{helpers::buildSpotLattice(spot, u, d, numNodes)}; // shape: [node, sigma]

    // Calculate payoff at expiration (intrinsic value only at expriation)
    torch::Tensor optionValues{helpers::intrinsicValue(expirationSpot.view({numNodes, 1, -1}), strikes,
                                                       isCall)}; // shape: [node, strike, sigma]

    // Backward induction
    for (std::int64_t depth{constants::trinomialDepth - 1}; depth > -1; --depth) {
        // Compute spot prices at this timestep
        const std::int64_t numNodesDepth{2 * depth + 1};
        const torch::Tensor spotsDepth{helpers::buildSpotLattice(spot, u, d, numNodesDepth)};

        // Select the relevant slice of the next timestep's optionValues
        const torch::Tensor nextOptionValues{optionValues.slice(0, 0, numNodesDepth + 2)}; // up, mid, down

        // Compute expected value using risk-neutral probabilities
        const torch::Tensor expectedValue{pU * nextOptionValues.slice(0, 2, numNodesDepth + 2) +
                                          pM * nextOptionValues.slice(0, 1, numNodesDepth + 1) +
                                          pD * nextOptionValues.slice(0, 0, numNodesDepth)};

        // Discount back one timestep
        const torch::Tensor continuationValue{discountFactor * expectedValue};

        // Compute intrinsic value
        const torch::Tensor exerciseValue{
            helpers::intrinsicValue(spotsDepth.view({numNodesDepth, 1, -1}), strikes, isCall)};

        // Update optionValues at this timestep
        optionValues = torch::max(exerciseValue, continuationValue);
    }

    // After backward induction, optionValues has shape [1, strikes, sigmas]
    return optionValues.squeeze(0); // remove the singleton node dimension
}

} // namespace greeks::trinomial::details