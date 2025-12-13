#pragma once

#include <torch/torch.h>
#include <utility>

namespace greeks {

struct GreeksResult {
    torch::Tensor price;
    torch::Tensor delta;
    torch::Tensor gamma;
    torch::Tensor vega;
    torch::Tensor theta;

    GreeksResult(torch::Tensor&& price_, torch::Tensor&& delta_, torch::Tensor&& gamma_, torch::Tensor&& vega_,
                 torch::Tensor&& theta_)
        : price{std::move(price_)}, delta{std::move(delta_)}, gamma{std::move(gamma_)}, vega{std::move(vega_)},
          theta{std::move(theta_)} {}
};

} // namespace greeks
