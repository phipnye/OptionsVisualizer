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

} // namespace models::trinomial::helpers
