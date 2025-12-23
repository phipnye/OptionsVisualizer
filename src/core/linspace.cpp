#include "OptionsVisualizer/core/linspace.hpp"
#include <Eigen/Dense>

Eigen::VectorXd linspace(const Eigen::DenseIndex size, const double lo, const double hi) {
    return Eigen::VectorXd::LinSpaced(size, lo, hi);
}
