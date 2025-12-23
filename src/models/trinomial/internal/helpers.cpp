#include "OptionsVisualizer/models/trinomial/internal/helpers.hpp"
#include <Eigen/Dense>
#include <utility>

namespace models::trinomial::helpers {

Eigen::MatrixXd buildSpotLattice(const double spot, const Eigen::VectorXd& u, const Eigen::DenseIndex depth,
                                 const Eigen::DenseIndex nSigma) {
    const Eigen::DenseIndex nNodes{2 * depth + 1};
    Eigen::MatrixXd lattice(nNodes, nSigma);

    for (Eigen::DenseIndex node{0}; node < nNodes; ++node) {
        // Each row is a node, columns are sigmas
        lattice.row(node) = spot * u.array().pow(static_cast<double>(node - depth));
    }

    return lattice;
}

} // namespace models::trinomial::helpers
