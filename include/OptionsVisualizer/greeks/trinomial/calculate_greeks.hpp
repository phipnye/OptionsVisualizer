#pragma once

#include "OptionsVisualizer/greeks/GreeksResult.hpp"
#include <torch/torch.h>

namespace greeks::trinomial {

GreeksResult callGreeks(torch::Tensor spot, torch::Tensor strikes, torch::Tensor r, torch::Tensor q,
                        torch::Tensor sigmas, torch::Tensor tau);

GreeksResult putGreeks(torch::Tensor spot, torch::Tensor strikes, torch::Tensor r, torch::Tensor q,
                       torch::Tensor sigmas, torch::Tensor tau);

} // namespace greeks::trinomial
