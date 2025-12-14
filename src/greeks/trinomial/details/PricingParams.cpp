#include "OptionsVisualizer/greeks/trinomial/details/PricingParams.hpp"
#include <torch/torch.h>
#include <utility>

namespace greeks::trinomial::details {

PricingParams::PricingParams(torch::Tensor&& dTau_, torch::Tensor&& u_, torch::Tensor&& d_,
                             torch::Tensor&& discountFactor_, torch::Tensor&& pU_, torch::Tensor&& pM_,
                             torch::Tensor&& pD_)
    : dTau{std::move(dTau_)}, u{std::move(u_)}, d{std::move(d_)}, discountFactor{std::move(discountFactor_)},
      pU{std::move(pU_)}, pM{std::move(pM_)}, pD{std::move(pD_)} {}

} // namespace greeks::trinomial::details