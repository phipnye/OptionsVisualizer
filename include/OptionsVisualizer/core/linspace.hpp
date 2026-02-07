#pragma once

#include <Eigen/Dense>

// Exported function for generating linearly separated points of size `size`
// between lo and hi (allowing for consistency between python and C++)
[[nodiscard]] Eigen::ArrayXd linspace(Eigen::Index size, double lo, double hi);
