#include "OptionsVisualizer/greeks/trinomial/details/calculate_greeks.hpp"
#include "OptionsVisualizer/greeks/trinomial/details/american_price.hpp"
#include <OptionsVisualizer/greeks/GreeksResult.hpp>
#include <torch/torch.h>
#include <utility>

namespace greeks::trinomial::details {

GreeksResult americanGreeks(torch::Tensor spot, torch::Tensor strikes, torch::Tensor r, torch::Tensor q,
                            torch::Tensor sigmas, torch::Tensor tau, bool isCall) {
    // Enable gradient tracking
    spot.requires_grad_(true);
    sigmas.requires_grad_(true);
    tau.requires_grad_(true);

    // Compute price
    torch::Tensor price{americanPrice(spot, strikes, r, q, sigmas, tau, isCall)};

    // --- First-order derivatives (delta, vega, theta)

    // Compute delta (first derivative w.r.t spot)
    torch::Tensor onesLikePrice{torch::ones_like(price)};
    torch::Tensor delta{torch::autograd::grad({price}, {spot}, {onesLikePrice},
                                              /* retain_graph */ true,
                                              /* allow_unused */ true,
                                              /* create_graph */ true)[0]};

    // Compute vega (first derivative w.r.t sigma)
    torch::Tensor vega{torch::autograd::grad({price}, {sigmas}, {onesLikePrice},
                                             /* retain_graph */ true,
                                             /* allow_unused */ true)[0]};

    // Compute theta (negative first derivative w.r.t tau)
    torch::Tensor theta{-torch::autograd::grad({price}, {tau}, {onesLikePrice},
                                               /* retain_graph */ true,
                                               /* allow_unused */ true)[0]};

    // --- Second-order derivatives (gamma)

    // Compute gamma (second derivative w.r.t spot)
    torch::Tensor gamma{torch::autograd::grad({delta}, {spot}, {onesLikePrice},
                                              /* retain_graph */ true,
                                              /* allow_unused */ true)[0]};

    return GreeksResult{std::move(price), std::move(delta), std::move(gamma), std::move(vega), std::move(theta)};
}

} // namespace greeks::trinomial::details
