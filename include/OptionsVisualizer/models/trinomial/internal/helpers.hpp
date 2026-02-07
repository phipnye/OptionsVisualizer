#pragma once

#include <Eigen/Dense>

#include "OptionsVisualizer/core/Enums.hpp"

namespace models::trinomial::helpers {

// Construct the spot lattice for a given depth (i.e., [spot * u^{-d}, spot *
// u^{-d+1}, ..., spot * u^{d-1}, spot * u^{d}])
[[nodiscard]] Eigen::ArrayXXd buildSpotLattice(double spot,
                                               const Eigen::ArrayXd& u,
                                               Eigen::Index depth);

// Compute the intrinsic value of a column vector of spot prices against a grid
// of strike prices
template <Enums::OptionType OptType, typename Derived>
[[nodiscard]] Eigen::ArrayXXd intrinsicValue(
    const Eigen::ArrayXXd& strikesGrid,
    const Eigen::ArrayBase<Derived>& spotsCol) {
  // Only permit for American option pricing
  static_assert(
      OptType == Enums::OptionType::AmerCall ||
          OptType == Enums::OptionType::AmerPut,
      "Intrinsic value computation only expected for American options");

  // Make sure spotsCol is a column vector
  static_assert(Derived::ColsAtCompileTime == 1,
                "Expected a column vector in 'intrinsicValue'");

  if constexpr (OptType == Enums::OptionType::AmerCall) {
    return (-(strikesGrid.colwise() - spotsCol)).cwiseMax(0.0);
  } else {
    return (strikesGrid.colwise() - spotsCol).cwiseMax(0.0);
  }
}

}  // namespace models::trinomial::helpers
