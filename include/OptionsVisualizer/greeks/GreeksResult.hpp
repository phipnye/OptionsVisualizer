#pragma once

#include <unsupported/Eigen/CXX11/Tensor>

namespace greeks {

struct GreeksResult {
    Eigen::Tensor<double, 2> price;
    Eigen::Tensor<double, 2> delta;
    Eigen::Tensor<double, 2> gamma;
    Eigen::Tensor<double, 2> vega;
    Eigen::Tensor<double, 2> theta;

    GreeksResult(Eigen::Tensor<double, 2>&& price_, Eigen::Tensor<double, 2>&& delta_,
                 Eigen::Tensor<double, 2>&& gamma_, Eigen::Tensor<double, 2>&& vega_,
                 Eigen::Tensor<double, 2>&& theta_);
};

} // namespace greeks