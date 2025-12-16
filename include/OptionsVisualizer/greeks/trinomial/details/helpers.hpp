#pragma once

#include "OptionsVisualizer/greeks/trinomial/details/PricingParams.hpp"
#include "OptionsVisualizer/grid/Dims.hpp"
#include <Eigen/Dense>
#include <unsupported/Eigen/CXX11/Tensor>

namespace greeks::trinomial::details::helpers {

PricingParams setupTrinomial(double r, double q, const Eigen::Tensor<double, 2>& sigmasGrid, double tau,
                             grid::index::Dims dims);

Eigen::Tensor<double, 2> buildSpotLattice(double spot, const Eigen::Tensor<double, 1>& u, Eigen::DenseIndex depth,
                                          grid::index::Dims dims);

Eigen::Tensor<double, 3> intrinsicValue(const Eigen::Tensor<double, 2>& spotsGrid,
                                        const Eigen::Tensor<double, 2>& strikesGrid, grid::index::Dims dims,
                                        bool isCall);

} // namespace greeks::trinomial::details::helpers