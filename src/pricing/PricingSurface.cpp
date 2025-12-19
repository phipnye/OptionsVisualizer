#pragma once

#include "OptionsVisualizer/pricing/PricingSurface.hpp"
#include "OptionsVisualizer/core/Enums.hpp"
#include "OptionsVisualizer/core/globals.hpp"
#include "OptionsVisualizer/core/linspace.hpp"
#include "OptionsVisualizer/pricing/GreeksResult.hpp"
#include <Eigen/Dense>
#include <array>

PricingSurface::PricingSurface(Eigen::DenseIndex nSigma, Eigen::DenseIndex nStrike, double spot, double r, double q,
                               double sigmaLo, double sigmaHi, double strikeLo, double strikeHi, double tau)
    : nSigma_{nSigma}, nStrike_{nStrike}, spot_{spot}, r_{r}, q_{q},
      sigmasGrid_{linspace(nSigma_, sigmaLo, sigmaHi).replicate(nStrike_, 1)},
      strikesGrid_{linspace(nStrike_, strikeLo, strikeHi).replicate(1, nSigma_)}, tau_{tau} {}

std::array<Eigen::MatrixXd, globals::nGrids> PricingSurface::calculateGrids() const {
    static const auto appendGreeks{
        [](std::array<Eigen::MatrixXd, globals::nGrids>& grids, Enums::OptionType optType, GreeksResult&& g) {
            constexpr std::size_t nGreeks{Enums::idx(Enums::GreekType::COUNT)};
            const std::size_t base{Enums::idx(optType) * nGreeks};
            grids[base + Enums::idx(Enums::GreekType::Price)] = std::move(g.price);
            grids[base + Enums::idx(Enums::GreekType::Delta)] = std::move(g.delta);
            grids[base + Enums::idx(Enums::GreekType::Gamma)] = std::move(g.gamma);
            grids[base + Enums::idx(Enums::GreekType::Vega)] = std::move(g.vega);
            grids[base + Enums::idx(Enums::GreekType::Theta)] = std::move(g.theta);
            grids[base + Enums::idx(Enums::GreekType::Rho)] = std::move(g.rho);
        }};

    // Generate results
    GreeksResult amerCall{trinomialGreeks(Enums::OptionType::AmerCall)};
    GreeksResult amerPut{trinomialGreeks(Enums::OptionType::AmerPut)};
    GreeksResult euroCall{bsmCallGreeks()};
    GreeksResult euroPut{bsmPutGreeks(euroCall)};

    // Move results to output array
    std::array<Eigen::MatrixXd, globals::nGrids> grids{};
    appendGreeks(grids, Enums::OptionType::AmerCall, std::move(amerCall));
    appendGreeks(grids, Enums::OptionType::AmerPut, std::move(amerPut));
    appendGreeks(grids, Enums::OptionType::EuroCall, std::move(euroCall));
    appendGreeks(grids, Enums::OptionType::EuroPut, std::move(euroPut));
    return grids;
}
