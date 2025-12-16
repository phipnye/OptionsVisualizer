#pragma once

#include "OptionsVisualizer/greeks/GreeksResult.hpp"
#include "OptionsVisualizer/grid/Dims.hpp"
#include <unsupported/Eigen/CXX11/Tensor>

namespace greeks::trinomial::details {

GreeksResult americanGreeks(double spot, const Eigen::Tensor<double, 2>& strikesGrid, double r, double q,
                            const Eigen::Tensor<double, 2>& sigmasGrid, double tau, grid::index::Dims dims,
                            bool isCall);

} // namespace greeks::trinomial::details
