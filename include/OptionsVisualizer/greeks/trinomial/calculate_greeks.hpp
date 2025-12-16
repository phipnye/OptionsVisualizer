#pragma once

#include "OptionsVisualizer/greeks/GreeksResult.hpp"
#include "OptionsVisualizer/grid/Dims.hpp"
#include <unsupported/Eigen/CXX11/Tensor>

namespace greeks::trinomial {

GreeksResult callGreeks(double spot, const Eigen::Tensor<double, 2>& strikesGrid, double r, double q,
                        const Eigen::Tensor<double, 2>& sigmasGrid, double tau, grid::index::Dims dims);

GreeksResult putGreeks(double spot, const Eigen::Tensor<double, 2>& strikesGrid, double r, double q,
                       const Eigen::Tensor<double, 2>& sigmasGrid, double tau, grid::index::Dims dims);

} // namespace greeks::trinomial
