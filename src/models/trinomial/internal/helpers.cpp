#include "OptionsVisualizer/models/trinomial/internal/helpers.hpp"

#include <Eigen/Dense>
#include <utility>

namespace models::trinomial::helpers {

Eigen::ArrayXXd buildSpotLattice(const double spot, const Eigen::ArrayXd& u,
                                 const Eigen::Index depth) {
  // Create the exponents: [-depth, ..., 0, ..., depth]
  const Eigen::Index nNodes{2 * depth + 1};
  const double depDbl{static_cast<double>(depth)};
  Eigen::ArrayXd exponents{Eigen::ArrayXd::LinSpaced(nNodes, -depDbl, depDbl)};

  // Generate a 2D grid where each cell (i, j) = spot * u[j]^{exponents[i]}
  // using the identity u^k = exp(k * ln(u)) via matrix outer product for speed
  return spot *
         (exponents.matrix() * u.log().matrix().transpose()).array().exp();
}

}  // namespace models::trinomial::helpers
