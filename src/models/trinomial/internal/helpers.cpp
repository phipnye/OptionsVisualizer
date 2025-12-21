#include "OptionsVisualizer/models/trinomial/internal/helpers.hpp"
#include "OptionsVisualizer/core/Enums.hpp"
#include <Eigen/Dense>
#include <cstddef>
#include <stdexcept>
#include <utility>
#include <vector>

namespace models::trinomial::helpers {

Eigen::MatrixXd buildSpotLattice(double spot, const Eigen::VectorXd& u, Eigen::DenseIndex depth,
                                 Eigen::DenseIndex nSigma) {
    const Eigen::DenseIndex nNodes{2 * depth + 1};
    Eigen::MatrixXd lattice(nNodes, nSigma);

    for (Eigen::DenseIndex i{0}; i < nNodes; ++i) {
        // Each row is a node, columns are sigmas
        lattice.row(i) = spot * u.array().pow(static_cast<double>(i - depth));
    }

    return lattice;
}

std::vector<Eigen::MatrixXd> intrinsicValue(const Eigen::MatrixXd& spotsGrid, const Eigen::MatrixXd& strikesGrid,
                                            Eigen::DenseIndex nNodes, Eigen::DenseIndex nSigma,
                                            Eigen::DenseIndex nStrike, Enums::OptionType optType) {
    std::vector<Eigen::MatrixXd> intrinsic(static_cast<std::size_t>(nNodes), Eigen::MatrixXd(nSigma, nStrike));

    switch (optType) {
    case Enums::OptionType::AmerCall:
        for (Eigen::DenseIndex i{0}; i < nNodes; ++i) {
            // spotsGrid is [nNodes x nSigma], row(i) gives us the sigmas for this node: [1 x nSigma]
            // To subtract this from a [nSigma x nStrike] matrix, treat it as a column vector [nSigma x 1] to
            // broadcast across the strikes
            const auto currentSpotsCol{spotsGrid.row(i).transpose()}; // auto gives expresion template

            // Payoff: max(S - K, 0) (subtract spots from every strike column, then negating to get S - K)
            intrinsic[i] = (-(strikesGrid.colwise() - currentSpotsCol)).cwiseMax(0.0);
        }
        break;

    case Enums::OptionType::AmerPut:
        for (Eigen::DenseIndex i{0}; i < nNodes; ++i) {
            const auto currentSpotsCol{spotsGrid.row(i).transpose()};

            // Payoff: max(K - S, 0) (subtracts the column vector from every strike column)
            intrinsic[i] = (strikesGrid.colwise() - currentSpotsCol).cwiseMax(0.0);
        }
        break;

    default:
        throw std::logic_error("Unhandled Enums::OptionType");
    }

    return intrinsic;
}

} // namespace models::trinomial::helpers
