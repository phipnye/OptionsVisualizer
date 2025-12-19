#include "OptionsVisualizer/pricing/GreeksResult.hpp"
#include <Eigen/Dense>
#include <utility>

explicit GreeksResult::GreeksResult(Eigen::MatrixXd&& price_, Eigen::MatrixXd&& delta_, Eigen::MatrixXd&& gamma_,
                                    Eigen::MatrixXd&& vega_, Eigen::MatrixXd&& theta_, Eigen::MatrixXd&& rho_)
    : price{std::move(price_)}, delta{std::move(delta_)}, gamma{std::move(gamma_)}, vega{std::move(vega_)},
      theta{std::move(theta_)}, rho{std::move(rho_)} {}
