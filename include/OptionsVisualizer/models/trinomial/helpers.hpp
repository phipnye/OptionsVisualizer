#pragma once

#include "OptionsVisualizer/Grid/Enums.hpp"
#include <unsupported/Eigen/CXX11/Tensor>

namespace models::trinomial::helpers {

Eigen::Tensor<double, 2> buildSpotLattice(double spot, const Eigen::Tensor<double, 1>& u, Eigen::DenseIndex depth,
                                          Eigen::DenseIndex nSigma);

Eigen::Tensor<double, 3> intrinsicValue(const Eigen::Tensor<double, 2>& spotsGrid,
                                        const Eigen::Tensor<double, 2>& strikesGrid, Eigen::DenseIndex nNodes,
                                        Eigen::DenseIndex nSigma, Eigen::DenseIndex nStrike, OptionType optType);

} // namespace models::trinomial::helpers
