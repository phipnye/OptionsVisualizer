#include "OptionsVisualizer/greeks/trinomial/details/helpers.hpp"
#include "OptionsVisualizer/greeks/trinomial/details/PricingParams.hpp"
#include "OptionsVisualizer/greeks/trinomial/details/constants.hpp"
#include "OptionsVisualizer/grid/Dims.hpp"
#include <Eigen/Dense>
#include <unsupported/Eigen/CXX11/Tensor>
#include <utility>

namespace greeks::trinomial::details::helpers {

PricingParams setupTrinomial(double r, double q, const Eigen::Tensor<double, 2>& sigmasGrid, double tau,
                             grid::index::Dims dims) {
    // Discrete time steps
    const double dTau{tau / constants::trinomialDepth};

    // Calculate u based on the first column of sigmasGrid
    // Stock price multipliers: u = e^(sigma * sqrt(3dt)); d = 1 / u
    Eigen::Tensor<double, 1> u{(sigmasGrid.chip(0, 1) * std::sqrt(3.0 * dTau)).exp()};

    // Single-step discount factor: discountFactor = e^(-r * dt)
    const double discountFactor{std::exp(-r * dTau)};

    // --- Intermediate risk-neutral probability terms (see Hull - Ch.20 (444))

    // Drift factor scaling term: sqrt(dt / 12 * sigma^2)
    const Eigen::Tensor<double, 2> scalingTerm{(dTau / (12.0 * sigmasGrid.square())).sqrt()};

    // Log stock drift: r - q * sigma^2 / 2
    const Eigen::Tensor<double, 2> logStockDrift{(r - q) - (sigmasGrid.square() / 2.0)};

    // Risk-neutral drift factor
    const Eigen::Tensor<double, 2> driftFactor{scalingTerm * logStockDrift};

    // --- Risk-neutral probabilities

    // Probability of upward price movement: p_u = sqrt(dt / 12 * sigma^2) * (r - q * sigma^2 / 2) + 1 / 6
    Eigen::Tensor<double, 3> pU{
        (driftFactor + (1.0 / 6.0)).reshape(Eigen::array<Eigen::DenseIndex, 3>{1, dims.nSigma, dims.nStrike})};

    // Probability of downward price movement: p_d = -sqrt(dt / 12 * sigma^2) * (r - q * sigma^2 / 2) + 1 / 6
    Eigen::Tensor<double, 3> pD{
        (-driftFactor + (1.0 / 6.0)).reshape(Eigen::array<Eigen::DenseIndex, 3>{1, dims.nSigma, dims.nStrike})};

    // p_m = 1 - p_u - p_d approx= 2/3
    Eigen::Tensor<double, 3> pM{1.0 - pU - pD};

    return PricingParams{std::move(u), discountFactor, std::move(pU), std::move(pM), std::move(pD)};
}

Eigen::Tensor<double, 2> buildSpotLattice(double spot, const Eigen::Tensor<double, 1>& u, Eigen::DenseIndex depth,
                                          grid::index::Dims dims) {
    const Eigen::DenseIndex numNodes{2 * depth + 1};
    Eigen::Tensor<double, 2> lattice(numNodes, dims.nSigma);

    for (Eigen::DenseIndex i{0}; i < numNodes; ++i) {
        lattice.chip(i, 0) = spot * u.pow(static_cast<double>(i - depth));
    }

    return lattice;
}

Eigen::Tensor<double, 3> intrinsicValue(const Eigen::Tensor<double, 2>& spotsGrid,
                                        const Eigen::Tensor<double, 2>& strikesGrid, grid::index::Dims dims,
                                        bool isCall) {
    const Eigen::DenseIndex nNodes{spotsGrid.dimension(0)};
    const auto [nSigma, nStrike]{dims};

    // Reshape to [nNodes, nSigma, 1]
    const Eigen::Tensor<double, 3> spots3D{spotsGrid.reshape(Eigen::array<Eigen::DenseIndex, 3>{nNodes, nSigma, 1})};
    // Reshape to [1, nSigma, nStrike]
    const Eigen::Tensor<double, 3> strikes3D{
        strikesGrid.reshape(Eigen::array<Eigen::DenseIndex, 3>{1, nSigma, nStrike})};

    // Spots broadcast across the nStrike dimension (dimension 2)
    const Eigen::array<Eigen::DenseIndex, 3> spotsBcast{1, 1, nStrike};
    // Strikes broadcast across the nNodes dimensio (dimension 0)
    const Eigen::array<Eigen::DenseIndex, 3> strikesBcast{nNodes, 1, 1};

    if (isCall) {
        // S - K
        return (spots3D.broadcast(spotsBcast) - strikes3D.broadcast(strikesBcast)).cwiseMax(0.0);
    }

    // K - S
    return (strikes3D.broadcast(strikesBcast) - spots3D.broadcast(spotsBcast)).cwiseMax(0.0);
}

} // namespace greeks::trinomial::details::helpers
