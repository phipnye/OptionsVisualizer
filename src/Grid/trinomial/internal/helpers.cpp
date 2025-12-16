#include "OptionsVisualizer/models/trinomial/helpers.hpp"
#include "OptionsVisualizer/Grid/Enums.hpp"
#include <Eigen/Dense>
#include <stdexcept>
#include <unsupported/Eigen/CXX11/Tensor>
#include <utility>

namespace models::trinomial::helpers {

Eigen::Tensor<double, 2> buildSpotLattice(double spot, const Eigen::Tensor<double, 1>& u, Eigen::DenseIndex depth,
                                          Eigen::DenseIndex nSigma) {
    const Eigen::DenseIndex numNodes{2 * depth + 1};
    Eigen::Tensor<double, 2> lattice(numNodes, nSigma);

    for (Eigen::DenseIndex i{0}; i < numNodes; ++i) {
        lattice.chip(i, 0) = spot * u.pow(static_cast<double>(i - depth));
    }

    return lattice;
}

Eigen::Tensor<double, 3> intrinsicValue(const Eigen::Tensor<double, 2>& spotsGrid,
                                        const Eigen::Tensor<double, 2>& strikesGrid, Eigen::DenseIndex nNodes,
                                        Eigen::DenseIndex nSigma, Eigen::DenseIndex nStrike, OptionType optType) {
    // Reshape to [nNodes, nSigma, 1]
    const Eigen::Tensor<double, 3> spots3D{spotsGrid.reshape(Eigen::array<Eigen::DenseIndex, 3>{nNodes, nSigma, 1})};
    // Reshape to [1, nSigma, nStrike]
    const Eigen::Tensor<double, 3> strikes3D{
        strikesGrid.reshape(Eigen::array<Eigen::DenseIndex, 3>{1, nSigma, nStrike})};

    // Spots broadcast across the nStrike dimension (dimension 2)
    const Eigen::array<Eigen::DenseIndex, 3> spotsBcast{1, 1, nStrike};
    // Strikes broadcast across the nNodes dimensio (dimension 0)
    const Eigen::array<Eigen::DenseIndex, 3> strikesBcast{nNodes, 1, 1};

    switch (optType) {
    case OptionType::AmerCall:
        // S - K
        return (spots3D.broadcast(spotsBcast) - strikes3D.broadcast(strikesBcast)).cwiseMax(0.0);

    case OptionType::AmerPut:
        // K - S
        return (strikes3D.broadcast(strikesBcast) - spots3D.broadcast(spotsBcast)).cwiseMax(0.0);

    default:
        throw std::logic_error("Unhandled Grid::OptionType");
    }
}

} // namespace models::trinomial::helpers
