#include "OptionsVisualizer/greeks/GreeksResult.hpp"
#include "OptionsVisualizer/greeks/bsm/calculate_greeks.hpp"
#include "OptionsVisualizer/greeks/trinomial/calculate_greeks.hpp"
#include "OptionsVisualizer/grid/Dims.hpp"
#include "OptionsVisualizer/grid/calculate_grids.hpp"
#include "OptionsVisualizer/grid/index.hpp"
#include <Eigen/Dense>
#include <iostream>
#include <unsupported/Eigen/CXX11/Tensor>
#include <vector>

int main() {
    // --- Hard-coded parameters
    double spot = 100.0;
    double r = 0.05;
    double q = 0.0;
    double tau = 0.5;

    const int nSigma = 10;
    const int nStrike = 10;

    // --- Hard-coded 1D grids (using std::vector as before)
    std::vector<double> strike_values = {80.000000,  84.444444,  88.888889,  93.333333,  97.777778,
                                         102.222222, 106.666667, 111.111111, 115.555556, 120.000000};

    std::vector<double> sigma_values = {0.100000, 0.133333, 0.166667, 0.200000, 0.233333,
                                        0.266667, 0.300000, 0.333333, 0.366667, 0.400000};

    // --- Convert std::vector to Eigen::VectorXd to satisfy the function signature
    // Eigen::Ref needs an object it can reference, so we use a Map or create a full VectorXd.
    // Creating a VectorXd for simplicity here.
    Eigen::VectorXd strikes_vec = Eigen::Map<Eigen::VectorXd>(strike_values.data(), nStrike);
    Eigen::VectorXd sigmas_vec = Eigen::Map<Eigen::VectorXd>(sigma_values.data(), nSigma);

    // --- Call the modular function
    // The function signature expects (double, Eigen::Ref<VectorXd>, double, double, Eigen::Ref<VectorXd>, double)
    grid::calculateGrids(spot, strikes_vec, r, q, sigmas_vec, tau);
    return 0;
}