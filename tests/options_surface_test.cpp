#include <Eigen/Dense>
#include <iostream>

#include "OptionsVisualizer/core/Enums.hpp"
#include "OptionsVisualizer/core/OptionsManager.hpp"
#include "OptionsVisualizer/core/globals.hpp"

int main() {
  std::size_t lruCapacity{5};
  OptionsManager manager{lruCapacity};
  Eigen::DenseIndex nSigma{10};
  Eigen::DenseIndex nStrike{10};
  double spot{100.0};
  double r{0.05};
  double q{0.02};
  double tau{1.0};
  double sigmaLo{0.1};
  double sigmaHi{0.4};
  double strikeLo{80.0};
  double strikeHi{120.0};

  try {
    const std::array<Eigen::MatrixXd, 24UL>& grids{
        manager.get(nSigma, nStrike, spot, r, q, sigmaLo, sigmaHi, strikeLo,
                    strikeHi, tau)};

    for (std::size_t idx{0}; idx < grids.size(); ++idx) {
      std::cout << grids[idx] << "\n\n";
    }

  } catch (const std::exception& e) {
    std::cerr << e.what() << "\n";
    return 1;
  }

  return 0;
}