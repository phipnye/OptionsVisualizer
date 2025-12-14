#include "OptionsVisualizer/greeks/trinomial/details/helpers.hpp"
#include "OptionsVisualizer/greeks/trinomial/details/constants.hpp"
#include "OptionsVisualizer/greeks/trinomial/details/PricingParams.hpp"
#include <torch/torch.h>

namespace greeks::trinomial::details::helpers {

PricingParams setupTrinomial(const torch::Tensor& r, const torch::Tensor& q, const torch::Tensor& sigmas,
                             const torch::Tensor& tau) {
    // Discrete time steps
    torch::Tensor dTau{tau / constants::trinomialDepth};

    // Stock price multipliers: u = e^(sigma * sqrt(3dt)); d = 1 / u
    torch::Tensor u{torch::exp(sigmas * torch::sqrt(3.0 * dTau)).view({1, -1})}; // [1, N]
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

    return PricingParams{std::move(dTau), std::move(u),  std::move(d), std::move(discountFactor),
                         std::move(pU),   std::move(pM), std::move(pD)};
}

torch::Tensor buildSpotLattice(const torch::Tensor& spot, const torch::Tensor& u, const torch::Tensor& d,
                               std::int64_t numNodes) {
    const torch::Tensor exponents{torch::arange(numNodes, torch::kInt64).view({numNodes, 1})};
    return spot * torch::pow(d, numNodes - exponents - 1) * torch::pow(u, exponents);
}

torch::Tensor intrinsicValue(const torch::Tensor& spotLattice, const torch::Tensor& strikes, bool isCall) {
    return torch::relu(isCall ? spotLattice - strikes.view({1, -1, 1}) : strikes.view({1, -1, 1}) - spotLattice);
}

} // namespace greeks::trinomial::details::helpers