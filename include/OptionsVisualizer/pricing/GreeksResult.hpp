#pragma once

#include <Eigen/Dense>

// Container for holding greeks calculations
struct GreeksResult {
  Eigen::ArrayXXd price_;
  Eigen::ArrayXXd delta_;
  Eigen::ArrayXXd gamma_;
  Eigen::ArrayXXd vega_;
  Eigen::ArrayXXd theta_;
  Eigen::ArrayXXd rho_;

  explicit GreeksResult(Eigen::ArrayXXd&& price, Eigen::ArrayXXd&& delta,
                        Eigen::ArrayXXd&& gamma, Eigen::ArrayXXd&& vega,
                        Eigen::ArrayXXd&& theta, Eigen::ArrayXXd&& rho);
};
