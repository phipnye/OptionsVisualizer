#include "OptionsVisualizer/models/constants.hpp"
#include <torch/torch.h>

namespace pricing {

struct TrinomialParams {
    torch::Tensor dTau;
    torch::Tensor u;
    torch::Tensor d;
    torch::Tensor discountFactor;
    torch::Tensor pU;
    torch::Tensor pM;
    torch::Tensor pD;

    TrinomialParams(torch::Tensor&& dTau_, torch::Tensor&& u_, torch::Tensor&& d_, torch::Tensor&& discountFactor_,
                    torch::Tensor&& pU_, torch::Tensor&& pM_, torch::Tensor&& pD_)
        : dTau{std::move(dTau_)}, u{std::move(u_)}, d{std::move(d_)}, discountFactor{std::move(discountFactor_)},
          pU{std::move(pU_)}, pM{std::move(pM_)}, pD{std::move(pD_)} {}
};

TrinomialParams setupTrinomial(const torch::Tensor& r, const torch::Tensor& q, const torch::Tensor& sigmas,
                               const torch::Tensor& tau) {
    // Discrete time steps
    torch::Tensor dTau{tau / constants::trinomialDepth};

    // Stock price multipliers: u = e^(sigma * sqrt(3dt)); d = 1 / u
    torch::Tensor u{torch::exp(sigmas * torch::sqrt(3.0 * dTau))};
    torch::Tensor d{torch::reciprocal(u)};

    // Single-step discount factor: discountFactor = e^(-r * dt)
    torch::Tensor discountFactor{torch::exp(-r * dTau)};

    // --- Intermediate risk-neutral probability terms (see Hull - Ch.20 (444))

    // Drift factor scaling term: sqrt(dt / 12 * sigma^2)
    torch::Tensor scalingTerm{torch::sqrt(dTau / (12.0 * sigmas.pow(2)))};

    // Log stock drift: r - q * sigma^2 / 2
    torch::Tensor logStockDrift{r - q - (sigmas.pow(2) / 2.0)};

    // Risk-neutral drift factor
    torch::Tensor driftFactor{scalingTerm * logStockDrift};

    // --- Risk-neutral probabilities

    // Probability of upward price movement: p_u = sqrt(dt / 12 * sigma^2) * (r - q * sigma^2 / 2) + 1 / 6
    torch::Tensor pU{driftFactor + (1.0 / 6.0)};

    // Probability of downward price movement: p_d = -sqrt(dt / 12 * sigma^2) * (r - q * sigma^2 / 2) + 1 / 6
    torch::Tensor pD{-driftFactor + 1.0 / 6.0};

    // p_m = 1 - p_u - p_d approx= 2/3
    torch::Tensor pM{1.0 - pU - pD};

    return TrinomialParams{std::move(dTau), std::move(u),  std::move(d), std::move(discountFactor),
                           std::move(pU),   std::move(pM), std::move(pD)};
}

/**
 * @brief Prices an American option using the trinomial model with LibTorch Tensors.
 */
torch::Tensor trinomialPrice(const torch::Tensor& spot, const torch::Tensor& strikes, const torch::Tensor& r,
                             const torch::Tensor& q, const torch::Tensor& sigmas, const torch::Tensor& tau,
                             bool isCall) {
    // Get setup terms
    const auto [dTau, u, d, discountFactor, pU, pM, pD]{setupTrinomial(r, q, sigmas, tau)};

    // Broadcast strikes x sigmas
    const torch::Tensor strikesExpanded{strikes.unsqueeze(1)}; // (N, 1)
    const torch::Tensor sigmasExpanded{sigmas.unsqueeze(0)};   // (1, N)
    const torch::Tensor sigmaGrid{sigmasExpanded.expand({strikes.size(0), sigmas.size(0)})};
    const torch::Tensor strikeGrid{strikesExpanded.expand_as(sigmaGrid)};

    // --- Compute price using backward induction

    // Calculate option values at expiration
    std::int64_t numNodes{2 * constants::trinomialDepth - 1};
    torch::Tensor exponents{torch::arange(numNodes, torch::kInt)}; // [0, 1, 2, ... , 2 * depth]
    torch::Tensor expirationSpot{spot * torch::pow(d, constants::trinomialDepth - exponents) *
                                 torch::pow(u, exponents)};
    torch::Tensor optionValues{expirationSpot.view({numNodes, 1, 1}) -
                               strikes.view({1, -1, 1})}; // shape: node x strike x sigma

    // Calculate payoff at expiration (American option: intrinsic value only)
    if (isCall) {
        optionValues = torch::relu(optionValues); // call: max(S - K, 0)
    } else {
        optionValues = torch::relu(-optionValues); // put: max(K - S, 0)
    }

    // Backward induction
    for (std::int64_t t{constants::trinomialDepth - 1}; t > -1; --t) {
        std::int64_t numNodesAtT{2 * t + 1};

        // Compute spot prices at this timestep
        torch::Tensor exponentsT{torch::arange(numNodesAtT, torch::kInt)};
        torch::Tensor spotsT{spot * torch::pow(d, t - exponentsT) * torch::pow(u, exponentsT)};

        // Select the relevant slice of the next timestep's optionValues
        torch::Tensor nextOptionValues{optionValues.slice(0, 0, numNodesAtT + 2)}; // up, mid, down

        // Compute expected value using trinomial probabilities
        torch::Tensor EV{pU * nextOptionValues.slice(0, 2, numNodesAtT + 2) +
                         pM * nextOptionValues.slice(0, 1, numNodesAtT + 1) +
                         pD * nextOptionValues.slice(0, 0, numNodesAtT)};

        // Discount back one timestep
        torch::Tensor continuationValue{discountFactor * EV};

        // Compute intrinsic value
        torch::Tensor intrinsicValue;

        if (isCall) {
            intrinsicValue = torch::relu(spotsT.view({numNodesAtT, 1, 1}) - strikeGrid.slice(0, 0, strikes.size(0)));
        } else {
            intrinsicValue = torch::relu(strikeGrid.slice(0, 0, strikes.size(0)) - spotsT.view({numNodesAtT, 1, 1}));
        }

        // Update optionValues at this timestep
        optionValues = torch::max(intrinsicValue, continuationValue);
    }

    // After backward induction, optionValues has shape (1, strikes.size(), sigmas.size())
    return optionValues.squeeze(0); // remove the singleton node dimension
}

} // namespace pricing
