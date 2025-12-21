#pragma once

#include "OptionsVisualizer/core/Enums.hpp"
#include <Eigen/Dense>
#include <vector>

namespace models::trinomial::helpers {

Eigen::MatrixXd buildSpotLattice(double spot, const Eigen::VectorXd& u, Eigen::DenseIndex depth,
                                 Eigen::DenseIndex nSigma);

std::vector<Eigen::MatrixXd> intrinsicValue(const Eigen::MatrixXd& spotsGrid, const Eigen::MatrixXd& strikesGrid,
                                            Eigen::DenseIndex nNodes, Eigen::DenseIndex nSigma,
                                            Eigen::DenseIndex nStrike, Enums::OptionType optType);

} // namespace models::trinomial::helpers
