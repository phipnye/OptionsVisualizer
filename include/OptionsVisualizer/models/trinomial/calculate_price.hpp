#pragma once

#include "OptionsVisualizer/Grid/Enums.hpp"
#include <unsupported/Eigen/CXX11/Tensor>

namespace models::trinomial {

Eigen::Tensor<double, 2> calculatePrice(double spot, const Eigen::Tensor<double, 2>& strikesGrid, double r, double q,
                                        const Eigen::Tensor<double, 2>& sigmasGrid, double tau,
                                        Eigen::DenseIndex nSigma, Eigen::DenseIndex nStrike, OptionType optType);

} // namespace models::trinomial
