#include "OptionsVisualizer/pricing/PricingSurface.hpp"

#include <Eigen/Dense>
#include <array>

#include "BS_thread_pool.hpp"
#include "OptionsVisualizer/core/Enums.hpp"
#include "OptionsVisualizer/core/globals.hpp"
#include "OptionsVisualizer/core/linspace.hpp"
#include "OptionsVisualizer/pricing/GreeksResult.hpp"

PricingSurface::PricingSurface(const Eigen::Index nSigma,
                               const Eigen::Index nStrike, const double spot,
                               const double r, const double q,
                               const double sigmaLo, const double sigmaHi,
                               const double strikeLo, const double strikeHi,
                               const double tau, BS::thread_pool<>& pool)
    : sigmasGrid_{linspace(nSigma, sigmaLo, sigmaHi).replicate(1, nStrike)},
      strikesGrid_{linspace(nStrike, strikeLo, strikeHi)
                       .transpose()
                       .replicate(nSigma, 1)},
      nSigma_{nSigma},
      nStrike_{nStrike},
      spot_{spot},
      r_{r},
      q_{q},
      tau_{tau},
      pool_{pool} {}

std::array<Eigen::ArrayXXd, globals::nGrids> PricingSurface::calculateGrids()
    const {
  const auto appendGreeks{
      [](std::array<Eigen::ArrayXXd, globals::nGrids>& grids,
         const Enums::OptionType optType, GreeksResult&& g) {
        const std::size_t base{Enums::idx(optType) *
                               Enums::idx(Enums::GreekType::COUNT)};
        grids[base + Enums::idx(Enums::GreekType::Price)] = std::move(g.price_);
        grids[base + Enums::idx(Enums::GreekType::Delta)] = std::move(g.delta_);
        grids[base + Enums::idx(Enums::GreekType::Gamma)] = std::move(g.gamma_);
        grids[base + Enums::idx(Enums::GreekType::Vega)] = std::move(g.vega_);
        grids[base + Enums::idx(Enums::GreekType::Theta)] = std::move(g.theta_);
        grids[base + Enums::idx(Enums::GreekType::Rho)] = std::move(g.rho_);
      }};

  // Generate results
  GreeksResult amerCall{trinomialGreeks<Enums::OptionType::AmerCall>()};
  GreeksResult amerPut{trinomialGreeks<Enums::OptionType::AmerPut>()};
  GreeksResult euroCall{bsmCallGreeks()};
  GreeksResult euroPut{bsmPutGreeks(euroCall)};

  // Move results to output array
  std::array<Eigen::ArrayXXd, globals::nGrids> grids{};
  appendGreeks(grids, Enums::OptionType::AmerCall, std::move(amerCall));
  appendGreeks(grids, Enums::OptionType::AmerPut, std::move(amerPut));
  appendGreeks(grids, Enums::OptionType::EuroCall, std::move(euroCall));
  appendGreeks(grids, Enums::OptionType::EuroPut, std::move(euroPut));
  return grids;
}
