#pragma once

#include <Eigen/Dense>

#include "OptionsVisualizer/core/Enums.hpp"

namespace models::trinomial::helpers {

Eigen::MatrixXd buildSpotLattice(double spot, const Eigen::VectorXd& u,
                                 Eigen::DenseIndex depth,
                                 Eigen::DenseIndex nSigma);

template <Enums::OptionType T>
Eigen::MatrixXd intrinsicValue(const Eigen::MatrixXd& strikesGrid,
                               const auto& spotsCol) {
  // Only permit for American option pricing
  static_assert(
      T == Enums::OptionType::AmerCall || T == Enums::OptionType::AmerPut,
      "Intrinsic value computation only expected for American options");

  if constexpr (T == Enums::OptionType::AmerCall) {
    return (-(strikesGrid.colwise() - spotsCol)).cwiseMax(0.0);
  } else {
    return (strikesGrid.colwise() - spotsCol).cwiseMax(0.0);
  }
}

}  // namespace models::trinomial::helpers
