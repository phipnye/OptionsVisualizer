#pragma once

#include "OptionsVisualizer/Grid/Enums.hpp"
#include <Eigen/Dense>
#include <unsupported/Eigen/CXX11/Tensor>

class Grid {
    // --- Data-members
    const Eigen::DenseIndex nSigma_;
    const Eigen::DenseIndex nStrike_;
    const double spot_;
    const Eigen::Tensor<double, 2> strikesGrid_;
    const double r_;
    const double q_;
    const Eigen::Tensor<double, 2> sigmasGrid_;
    const double tau_;

    // --- Container for holding greeks calculations
    struct GreeksResult {
        Eigen::Tensor<double, 2> price;
        Eigen::Tensor<double, 2> delta;
        Eigen::Tensor<double, 2> gamma;
        Eigen::Tensor<double, 2> vega;
        Eigen::Tensor<double, 2> theta;

        GreeksResult(Eigen::Tensor<double, 2>&& price_, Eigen::Tensor<double, 2>&& delta_,
                     Eigen::Tensor<double, 2>&& gamma_, Eigen::Tensor<double, 2>&& vega_,
                     Eigen::Tensor<double, 2>&& theta_);
    };

    // --- Black-Scholes-Merton
    GreeksResult bsmCallGreeks() const;
    GreeksResult bsmPutGreeks(const GreeksResult& callResults) const;

    // --- Trinomial tree
    GreeksResult trinomialGreeks(OptionType optType) const;

    // --- Concatenate results into a single tensor
    void writeOptionGreeks(Eigen::VectorXd& output, OptionType optType, const GreeksResult& result) const;

public:
    Grid(double spot, Eigen::Ref<Eigen::VectorXd> strikes, double r, double q, Eigen::Ref<Eigen::VectorXd> sigmas,
         double tau);

    Eigen::VectorXd calculateGrids() const;
};
