#include "OptionsVisualizer/core/linspace.hpp"

#include <Eigen/Dense>

Eigen::ArrayXd linspace(const Eigen::Index size, const double lo,
                        const double hi) {
  return Eigen::ArrayXd::LinSpaced(size, lo, hi);
}
