#pragma once

#include "OptionsVisualizer/greeks/trinomial/details/PricingParams.hpp"
#include <torch/torch.h>

namespace greeks::trinomial::details::helpers {

PricingParams setupTrinomial(const torch::Tensor& r, const torch::Tensor& q, const torch::Tensor& sigmas,
                             const torch::Tensor& tau);

torch::Tensor buildSpotLattice(const torch::Tensor& spot, const torch::Tensor& u, const torch::Tensor& d,
                               std::int64_t numNodes);

torch::Tensor intrinsicValue(const torch::Tensor& spotLattice, const torch::Tensor& strikes, bool isCall);

} // namespace greeks::trinomial::details::helpers
