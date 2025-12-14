#pragma once

#include <torch/torch.h>

namespace greeks::trinomial::details {

torch::Tensor americanPrice(const torch::Tensor& spot, const torch::Tensor& strikes, const torch::Tensor& r,
                            const torch::Tensor& q, const torch::Tensor& sigmas, const torch::Tensor& tau, bool isCall);

} // namespace greeks::trinomial::details