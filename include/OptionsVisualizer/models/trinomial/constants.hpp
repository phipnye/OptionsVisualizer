#pragma once

#include <Eigen/Dense>

namespace models::trinomial::constants {

inline constexpr double fdmStep{1e-6};
inline constexpr Eigen::DenseIndex trinomialDepth{100};

} // namespace models::trinomial::constants
