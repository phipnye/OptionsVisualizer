#include "OptionsVisualizer/grid/calculate_grids.hpp"
#include "OptionsVisualizer/greeks/GreeksResult.hpp"
#include "OptionsVisualizer/greeks/bsm/calculate_greeks.hpp"
#include "OptionsVisualizer/greeks/trinomial/calculate_greeks.hpp"
#include "OptionsVisualizer/grid/Dims.hpp"
#include "OptionsVisualizer/grid/index.hpp"
#include <Eigen/Dense>
#include <pybind11/eigen.h>
#include <pybind11/pybind11.h>
#include <unsupported/Eigen/CXX11/Tensor>
namespace py = pybind11;

namespace grid {

Eigen::VectorXd calculateGrids(double spot, Eigen::Ref<Eigen::VectorXd> strikes, double r, double q,
                               Eigen::Ref<Eigen::VectorXd> sigmas, double tau) {
    // --- Setup
    const index::Dims dims{.nSigma{sigmas.size()}, .nStrike{strikes.size()}};

    // --- Broadcast strikes and sigmas
    const Eigen::array<Eigen::DenseIndex, 2> sigmaDims{dims.nSigma, 1};
    const Eigen::array<Eigen::DenseIndex, 2> strikeDims{1, dims.nStrike};

    // Map inputs to tensors (creates a view of the data without copying it yet)
    Eigen::TensorMap<Eigen::Tensor<double, 2>> sigmasTensorMapping{sigmas.data(), sigmaDims};
    Eigen::TensorMap<Eigen::Tensor<double, 2>> strikesTensorMapping{strikes.data(), strikeDims};

    // Broadcast strikes and sigmas (generates a copy at this point)
    const Eigen::Tensor<double, 2> sigmasGrid{sigmasTensorMapping.broadcast(strikeDims)};
    const Eigen::Tensor<double, 2> strikesGrid{strikesTensorMapping.broadcast(sigmaDims)};

    // Release GIL (data already copied no longer needed)
    py::gil_scoped_release noGil{};

    // Generate results
    greeks::GreeksResult amerCall{greeks::trinomial::callGreeks(spot, strikesGrid, r, q, sigmasGrid, tau, dims)};
    greeks::GreeksResult amerPut{greeks::trinomial::putGreeks(spot, strikesGrid, r, q, sigmasGrid, tau, dims)};
    greeks::GreeksResult euroCall{greeks::bsm::callGreeks(spot, strikesGrid, r, q, sigmasGrid, tau)};
    greeks::GreeksResult euroPut{greeks::bsm::putGreeks(spot, strikesGrid, r, q, sigmasGrid, tau)};

    // Write results to output
    py::gil_scoped_acquire acquireGil{};
    Eigen::VectorXd output(dims.nSigma * dims.nStrike * 4 * 5);
    index::writeOptionGreeks(output, index::OptionType::AmerCall, amerCall, dims);
    index::writeOptionGreeks(output, index::OptionType::AmerPut, amerPut, dims);
    index::writeOptionGreeks(output, index::OptionType::EuroCall, euroCall, dims);
    index::writeOptionGreeks(output, index::OptionType::EuroPut, euroPut, dims);
    return output;
}

} // namespace grid
