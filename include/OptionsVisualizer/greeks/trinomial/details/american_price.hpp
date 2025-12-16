#pragma once

#include "OptionsVisualizer/grid/Dims.hpp"
#include <unsupported/Eigen/CXX11/Tensor>

namespace greeks::trinomial::details {

Eigen::Tensor<double, 2> americanPrice(double spot, const Eigen::Tensor<double, 2>& strikesGrid, double r, double q,
                                       const Eigen::Tensor<double, 2>& sigmasGrid, double tau, grid::index::Dims dims,
                                       bool isCall);

} // namespace greeks::trinomial::details