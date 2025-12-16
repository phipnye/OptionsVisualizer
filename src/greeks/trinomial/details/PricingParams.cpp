#include "OptionsVisualizer/greeks/trinomial/details/PricingParams.hpp"
#include <unsupported/Eigen/CXX11/Tensor>
#include <utility>

namespace greeks::trinomial::details {

PricingParams::PricingParams(Eigen::Tensor<double, 1>&& u_, double discountFactor_, Eigen::Tensor<double, 3>&& pU_,
                             Eigen::Tensor<double, 3>&& pM_, Eigen::Tensor<double, 3>&& pD_)
    : u{std::move(u_)}, discountFactor{discountFactor_}, pU{std::move(pU_)}, pM{std::move(pM_)}, pD{std::move(pD_)} {}

} // namespace greeks::trinomial::details