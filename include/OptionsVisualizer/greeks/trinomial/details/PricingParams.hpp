#pragma once

#include <torch/torch.h>

namespace greeks::trinomial::details {

struct PricingParams {
    torch::Tensor dTau;
    torch::Tensor u;
    torch::Tensor d;
    torch::Tensor discountFactor;
    torch::Tensor pU;
    torch::Tensor pM;
    torch::Tensor pD;

    PricingParams(torch::Tensor&& dTau_, torch::Tensor&& u_, torch::Tensor&& d_, torch::Tensor&& discountFactor_,
                  torch::Tensor&& pU_, torch::Tensor&& pM_, torch::Tensor&& pD_);
};

} // namespace greeks::trinomial::details