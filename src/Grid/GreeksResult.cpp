#include "OptionsVisualizer/Grid/Grid.hpp"
#include <unsupported/Eigen/CXX11/Tensor>
#include <utility>

Grid::GreeksResult::GreeksResult(Eigen::Tensor<double, 2>&& price_, Eigen::Tensor<double, 2>&& delta_,
                                 Eigen::Tensor<double, 2>&& gamma_, Eigen::Tensor<double, 2>&& vega_,
                                 Eigen::Tensor<double, 2>&& theta_)
    : price{std::move(price_)}, delta{std::move(delta_)}, gamma{std::move(gamma_)}, vega{std::move(vega_)},
      theta{std::move(theta_)} {}
