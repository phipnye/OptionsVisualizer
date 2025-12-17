#include "OptionsVisualizer/Grid/Grid.hpp"
#include <Eigen/Dense>
#include <fstream>
#include <iostream>
#include <unsupported/Eigen/CXX11/Tensor>
#include <vector>

int main() {
    double spot{100.0};
    double r{0.05};
    double q{0.0};
    double tau{0.5};
    const Eigen::DenseIndex nSigma{10};
    const Eigen::DenseIndex nStrike{10};
    std::vector<double> strikesVec{80.000000,  84.444444,  88.888889,  93.333333,  97.777778,
                                   102.222222, 106.666667, 111.111111, 115.555556, 120.000000};

    std::vector<double> sigmasVec{0.100000, 0.133333, 0.166667, 0.200000, 0.233333,
                                  0.266667, 0.300000, 0.333333, 0.366667, 0.400000};

    Eigen::VectorXd strikes{Eigen::Map<Eigen::VectorXd>(strikesVec.data(), nStrike)};
    Eigen::VectorXd sigmas{Eigen::Map<Eigen::VectorXd>(sigmasVec.data(), nSigma)};
    Grid grid{spot, strikes, r, q, sigmas, tau};
    grid.calculateGrids();
    return 0;
}