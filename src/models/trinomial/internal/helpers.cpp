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

  // Use log math to create the grid: exp(log(u) * exponents)
  return spot *
         (exponents.matrix() * u.log().matrix().transpose()).array().exp();
}

}  // namespace models::trinomial::helpers
