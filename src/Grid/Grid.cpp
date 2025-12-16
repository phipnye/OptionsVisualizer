#include "OptionsVisualizer/Grid/Grid.hpp"
#include <Eigen/Dense>
#include <pybind11/pybind11.h>
#include <unsupported/Eigen/CXX11/Tensor>
namespace py = pybind11;

Grid::Grid(double spot, Eigen::Ref<Eigen::VectorXd> strikes, double r, double q, Eigen::Ref<Eigen::VectorXd> sigmas,
           double tau)
    : nSigma_{sigmas.size()}, nStrike_{strikes.size()}, spot_{spot},
      strikesGrid_{
          Eigen::TensorMap<Eigen::Tensor<double, 2>>{strikes.data(), Eigen::array<Eigen::DenseIndex, 2>{1, nStrike_}}
              .broadcast(Eigen::array<Eigen::DenseIndex, 2>{nSigma_, 1})},
      r_{r}, q_{q}, sigmasGrid_{Eigen::TensorMap<Eigen::Tensor<double, 2>>{
                        sigmas.data(), Eigen::array<Eigen::DenseIndex, 2>{nSigma_, 1}}
                                    .broadcast(Eigen::array<Eigen::DenseIndex, 2>{1, nStrike_})},
      tau_{tau} {}

Eigen::VectorXd Grid::calculateGrids() const {
    // Release GIL (data already copied no longer needed)
    py::gil_scoped_release noGil{};

    // Generate results
    GreeksResult amerCall{trinomialGreeks(OptionType::AmerCall)};
    GreeksResult amerPut{trinomialGreeks(OptionType::AmerPut)};
    GreeksResult euroCall{bsmCallGreeks()};
    GreeksResult euroPut{bsmPutGreeks()};

    // Write results to output
    py::gil_scoped_acquire acquireGil{};
    Eigen::VectorXd output(nSigma_ * nStrike_ * idx(OptionType::COUNT) * idx(GreekType::COUNT));
    writeOptionGreeks(output, OptionType::AmerCall, amerCall);
    writeOptionGreeks(output, OptionType::AmerPut, amerPut);
    writeOptionGreeks(output, OptionType::EuroCall, euroCall);
    writeOptionGreeks(output, OptionType::EuroPut, euroPut);
    return output;
}

void Grid::writeOptionGreeks(Eigen::VectorXd& output, OptionType optType, const Grid::GreeksResult& result) const {
    static constexpr Eigen::DenseIndex priceIdx{idx(GreekType::Price)};
    static constexpr Eigen::DenseIndex deltaIdx{idx(GreekType::Delta)};
    static constexpr Eigen::DenseIndex gammaIdx{idx(GreekType::Gamma)};
    static constexpr Eigen::DenseIndex vegaIdx{idx(GreekType::Vega)};
    static constexpr Eigen::DenseIndex thetaIdx{idx(GreekType::Theta)};
    const Eigen::DenseIndex optionIdx{idx(optType)};

    for (Eigen::DenseIndex sigmaIdx{0}; sigmaIdx < nSigma_; ++sigmaIdx) {
        for (Eigen::DenseIndex strikeIdx{0}; strikeIdx < nStrike_; ++strikeIdx) {
            const Eigen::DenseIndex base{sigmaIdx * nStrike_ * 4 * 5 + strikeIdx * 4 * 5 + optionIdx * 5};
            output(base + priceIdx) = result.price(sigmaIdx, strikeIdx);
            output(base + deltaIdx) = result.delta(sigmaIdx, strikeIdx);
            output(base + gammaIdx) = result.gamma(sigmaIdx, strikeIdx);
            output(base + vegaIdx) = result.vega(sigmaIdx, strikeIdx);
            output(base + thetaIdx) = result.theta(sigmaIdx, strikeIdx);
        }
    }
}