#pragma once

#include "OptionsVisualizer/core/Enums.hpp"
#include <Eigen/Dense>

namespace models::trinomial {

Eigen::MatrixXd calculatePrice(Eigen::DenseIndex nSigma, Eigen::DenseIndex nStrike, double spot, double r, double q,
                               const Eigen::MatrixXd& sigmasGrid, const Eigen::MatrixXd& strikesGrid, double tau,
                               Enums::OptionType optType);

} // namespace models::trinomial
