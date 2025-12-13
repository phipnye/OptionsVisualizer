#pragma once

#include "OptionsVisualizer/models/GreeksResult.hpp"
#include "OptionsVisualizer/models/trinomial_price.hpp"
#include <torch/torch.h>
#include <utility>

namespace greeks {

/**
 * @brief Calculates the price and major greeks (delta, gamma, vega, theta) for an American option using LibTorch
 * autograd.
 */
GreeksResult trinomialGreeks(torch::Tensor spot, torch::Tensor strikes, torch::Tensor r, torch::Tensor q,
                             torch::Tensor sigmas, torch::Tensor tau, bool isCall) {
    // Enable gradient tracking
    spot.requires_grad_(true);
    sigmas.requires_grad_(true);
    tau.requires_grad_(true);

    // Compute price
    torch::Tensor price{pricing::trinomialPrice(spot, strikes, r, q, sigmas, tau, isCall)};

    // Compute delta (first derivative w.r.t spot)
    torch::Tensor delta{torch::autograd::grad({price}, {spot}, {}, true, true)[0]};

    // Compute gamma (second derivative w.r.t spot)
    torch::Tensor gamma{torch::autograd::grad({delta}, {spot}, {}, true, true)[0]};

    // Compute vega (first derivative w.r.t sigma)
    torch::Tensor vega{torch::autograd::grad({price}, {sigmas}, {}, true, true)[0]};

    // Compute theta (negative derivative w.r.t tau)
    torch::Tensor theta{-torch::autograd::grad({price}, {tau}, {}, true, true)[0]};

    return GreeksResult{std::move(price), std::move(delta), std::move(gamma), std::move(vega), std::move(theta)};
}

/**
 * @brief Convenience wrapper for American call
 */
GreeksResult trinomialCallGreeks(torch::Tensor spot, torch::Tensor strikes, torch::Tensor r, torch::Tensor q,
                                 torch::Tensor sigmas, torch::Tensor tau) {
    return trinomialGreeks(spot, strikes, r, q, sigmas, tau, true);
}

/**
 * @brief Convenience wrapper for American put
 */
GreeksResult trinomialPutGreeks(torch::Tensor spot, torch::Tensor strikes, torch::Tensor r, torch::Tensor q,
                                torch::Tensor sigmas, torch::Tensor tau) {
    return trinomialGreeks(spot, strikes, r, q, sigmas, tau, false);
}

} // namespace greeks
