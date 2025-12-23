#pragma once

#include "OptionsVisualizer/core/Enums.hpp"
#include <Eigen/Dense>
#include <vector>

namespace models::trinomial::helpers {

Eigen::MatrixXd buildSpotLattice(double spot, const Eigen::VectorXd& u, Eigen::DenseIndex depth,
                                 Eigen::DenseIndex nSigma);

void intrinsicValue(std::vector<Eigen::MatrixXd>& exerciseValue, const Eigen::MatrixXd& spotsGrid,
                    const Eigen::MatrixXd& strikesGrid, Eigen::DenseIndex nNodes, Enums::OptionType optType);

} // namespace models::trinomial::helpers
