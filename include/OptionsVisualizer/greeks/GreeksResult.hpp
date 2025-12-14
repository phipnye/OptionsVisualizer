#pragma once

#include <torch/torch.h>

namespace greeks {

struct GreeksResult {
    torch::Tensor price;
    torch::Tensor delta;
    torch::Tensor gamma;
    torch::Tensor vega;
    torch::Tensor theta;

    GreeksResult(torch::Tensor&& price_, torch::Tensor&& delta_, torch::Tensor&& gamma_, torch::Tensor&& vega_,
                 torch::Tensor&& theta_);
};

} // namespace greeks
