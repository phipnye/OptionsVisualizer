#pragma once

#include <Eigen/Dense>

// Container for holding greeks calculations
struct GreeksResult {
    Eigen::MatrixXd price;
    Eigen::MatrixXd delta;
    Eigen::MatrixXd gamma;
    Eigen::MatrixXd vega;
    Eigen::MatrixXd theta;
    Eigen::MatrixXd rho;

    explicit GreeksResult(Eigen::MatrixXd&& price_, Eigen::MatrixXd&& delta_, Eigen::MatrixXd&& gamma_,
                          Eigen::MatrixXd&& vega_, Eigen::MatrixXd&& theta_, Eigen::MatrixXd&& rho_);

    // Prevent copies
    GreeksResult(const GreeksResult&) = delete;
    GreeksResult& operator=(const GreeksResult&) = delete;

    // Allow moves
    GreeksResult(GreeksResult&&) = default;
    GreeksResult& operator=(GreeksResult&&) = default;

    // Rule of 5
    ~GreeksResult() = default;
};
