#pragma once

#include <Eigen/Dense>

namespace grid::index {

struct Dims {
    Eigen::DenseIndex nSigma;  // volatility will be row index
    Eigen::DenseIndex nStrike; // strike column index
};

} // namespace grid::index
