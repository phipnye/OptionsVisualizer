#include <Eigen/Dense>
#include <iostream>

#include "OptionsVisualizer/core/OptionsManager.hpp"

int main() {
  constexpr std::size_t lruCapacity{5};
  OptionsManager manager{lruCapacity, 1};

  try {
    constexpr Eigen::DenseIndex nSigma{10};
    constexpr Eigen::DenseIndex nStrike{10};
    constexpr double spot{100.0};
    constexpr double r{0.05};
    constexpr double q{0.02};
    constexpr double tau{1.0};
    constexpr double sigmaLo{0.1};
    constexpr double sigmaHi{0.4};
    constexpr double strikeLo{80.0};
    constexpr double strikeHi{120.0};

    const auto& grids{manager.get(nSigma, nStrike, spot, r, q, sigmaLo, sigmaHi,
                                  strikeLo, strikeHi, tau)};

    for (std::size_t idx{0}; idx < grids.size(); ++idx) {
      std::cout << grids[idx] << "\n\n";
    }

  } catch (const std::exception& e) {
    std::cerr << e.what() << "\n";
    return 1;
  }

  return 0;
}