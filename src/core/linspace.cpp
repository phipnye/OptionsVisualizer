#include "OptionsVisualizer/core/linspace.hpp"
#include <Eigen/Dense>

Eigen::VectorXd linspace(Eigen::DenseIndex size, double lo, double hi) {
    return Eigen::VectorXd::LinSpaced(size, lo, hi);
}
