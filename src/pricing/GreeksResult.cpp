#include "OptionsVisualizer/pricing/GreeksResult.hpp"

#include <Eigen/Dense>
#include <utility>

GreeksResult::GreeksResult(Eigen::ArrayXXd&& price, Eigen::ArrayXXd&& delta,
                           Eigen::ArrayXXd&& gamma, Eigen::ArrayXXd&& vega,
                           Eigen::ArrayXXd&& theta, Eigen::ArrayXXd&& rho)
    : price_{std::move(price)},
      delta_{std::move(delta)},
      gamma_{std::move(gamma)},
      vega_{std::move(vega)},
      theta_{std::move(theta)},
      rho_{std::move(rho)} {}
