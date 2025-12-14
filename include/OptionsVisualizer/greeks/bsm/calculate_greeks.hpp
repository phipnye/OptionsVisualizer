#pragma once

#include "OptionsVisualizer/greeks/GreeksResult.hpp"
#include <torch/torch.h>

namespace greeks::bsm {

GreeksResult callGreeks(const torch::Tensor& spot, const torch::Tensor& strikes, const torch::Tensor& r,
                        const torch::Tensor& q, const torch::Tensor& sigmas, const torch::Tensor& tau);

GreeksResult putGreeks(const torch::Tensor& spot, const torch::Tensor& strikes, const torch::Tensor& r,
                       const torch::Tensor& q, const torch::Tensor& sigmas, const torch::Tensor& tau);

} // namespace greeks::bsm
