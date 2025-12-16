#pragma once

#include <Eigen/Dense>
#include <pybind11/eigen.h>
#include <unsupported/Eigen/CXX11/Tensor>

namespace grid {

Eigen::VectorXd calculateGrids(double spot, Eigen::Ref<Eigen::VectorXd> strikes, double r, double q,
                               Eigen::Ref<Eigen::VectorXd> sigmas, double tau);

} // namespace grid
