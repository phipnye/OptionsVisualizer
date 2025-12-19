#pragma once

#include "OptionsVisualizer/core/Enums.hpp"
#include "OptionsVisualizer/core/globals.hpp"
#include "OptionsVisualizer/pricing/GreeksResult.hpp"
#include <Eigen/Dense>
#include <array>

class PricingSurface {
    // --- Data-members
    const Eigen::DenseIndex nSigma_;
    const Eigen::DenseIndex nStrike_;
    const double spot_;
    const double r_;
    const double q_;
    const Eigen::MatrixXd sigmasGrid_;
    const Eigen::MatrixXd strikesGrid_;
    const double tau_;

    // --- Black-Scholes-Merton
    GreeksResult bsmCallGreeks() const;
    GreeksResult bsmPutGreeks(const GreeksResult& callResults) const;

    // --- Trinomial tree
    GreeksResult trinomialGreeks(Enums::OptionType optType) const;

public:
    PricingSurface(Eigen::DenseIndex nSigma, Eigen::DenseIndex nStrike, double spot, double r, double q, double sigmaLo,
                   double sigmaHi, double strikeLo, double strikeHi, double tau);

    // Compute grids for all greeks and option types
    std::array<Eigen::MatrixXd, globals::nGrids> calculateGrids() const;
};
