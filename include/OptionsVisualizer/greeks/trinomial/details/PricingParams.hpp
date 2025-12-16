#pragma once

#include <unsupported/Eigen/CXX11/Tensor>

namespace greeks::trinomial::details {

struct PricingParams {
    Eigen::Tensor<double, 1> u;
    double discountFactor;
    Eigen::Tensor<double, 3> pU;
    Eigen::Tensor<double, 3> pM;
    Eigen::Tensor<double, 3> pD;

    PricingParams(Eigen::Tensor<double, 1>&& u_, double discountFactor_, Eigen::Tensor<double, 3>&& pU_,
                  Eigen::Tensor<double, 3>&& pM_, Eigen::Tensor<double, 3>&& pD_);
};

} // namespace greeks::trinomial::details