#include "OptionsVisualizer/Grid/Grid.hpp"
#include <Eigen/Dense>
#include <iostream>
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
#ifdef NDEBUG
    py::gil_scoped_release noGil{};
#endif

    // Generate results
    GreeksResult amerCall{trinomialGreeks(OptionType::AmerCall)};
    GreeksResult amerPut{trinomialGreeks(OptionType::AmerPut)};
    GreeksResult euroCall{bsmCallGreeks()};
    GreeksResult euroPut{bsmPutGreeks(euroCall)};

    // Write results to output
#ifdef NDEBUG
    py::gil_scoped_acquire acquireGil{};
#endif
    Eigen::VectorXd output(nSigma_ * nStrike_ * idx(OptionType::COUNT) * idx(GreekType::COUNT));
    writeOptionGreeks(output, OptionType::AmerCall, amerCall);
    writeOptionGreeks(output, OptionType::AmerPut, amerPut);
    writeOptionGreeks(output, OptionType::EuroCall, euroCall);
    writeOptionGreeks(output, OptionType::EuroPut, euroPut);
    return output;
}

void Grid::writeOptionGreeks(Eigen::VectorXd& output, OptionType optType, const Grid::GreeksResult& result) const {
    constexpr Eigen::DenseIndex nOptions{idx(OptionType::COUNT)};

    constexpr Eigen::DenseIndex priceIdx{idx(GreekType::Price)};
    constexpr Eigen::DenseIndex deltaIdx{idx(GreekType::Delta)};
    constexpr Eigen::DenseIndex gammaIdx{idx(GreekType::Gamma)};
    constexpr Eigen::DenseIndex vegaIdx{idx(GreekType::Vega)};
    constexpr Eigen::DenseIndex thetaIdx{idx(GreekType::Theta)};
    constexpr Eigen::DenseIndex rhoIdx{idx(GreekType::Rho)};

    const Eigen::DenseIndex optionIdx{idx(optType)};
    const Eigen::DenseIndex optionStride{nSigma_ * nStrike_};
    const Eigen::DenseIndex greekStride{nSigma_ * nStrike_ * nOptions};

    for (Eigen::DenseIndex sigmaIdx{0}; sigmaIdx < nSigma_; ++sigmaIdx) {
        for (Eigen::DenseIndex strikeIdx{0}; strikeIdx < nStrike_; ++strikeIdx) {
            // Column-major base: sigma is fastest, then strike, then option type, then greek type
            const Eigen::DenseIndex base{sigmaIdx + (strikeIdx * nSigma_) + (optionIdx * optionStride)};

            output(base + (priceIdx * greekStride)) = result.price(sigmaIdx, strikeIdx);
            output(base + (deltaIdx * greekStride)) = result.delta(sigmaIdx, strikeIdx);
            output(base + (gammaIdx * greekStride)) = result.gamma(sigmaIdx, strikeIdx);
            output(base + (vegaIdx * greekStride)) = result.vega(sigmaIdx, strikeIdx);
            output(base + (thetaIdx * greekStride)) = result.theta(sigmaIdx, strikeIdx);
            output(base + (rhoIdx * greekStride)) = result.rho(sigmaIdx, strikeIdx);
        }
    }
}