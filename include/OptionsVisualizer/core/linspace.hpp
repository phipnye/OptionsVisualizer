#pragma once

#include <Eigen/Dense>

[[nodiscard]] Eigen::VectorXd linspace(Eigen::DenseIndex size, double lo, double hi);
