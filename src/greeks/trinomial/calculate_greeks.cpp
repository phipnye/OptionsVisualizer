#include "OptionsVisualizer/greeks/trinomial/calculate_greeks.hpp"
#include "OptionsVisualizer/greeks/GreeksResult.hpp"
#include "OptionsVisualizer/greeks/trinomial/details/calculate_greeks.hpp"
#include "OptionsVisualizer/grid/Dims.hpp"
#include <unsupported/Eigen/CXX11/Tensor>

namespace greeks::trinomial {

GreeksResult callGreeks(double spot, const Eigen::Tensor<double, 2>& strikesGrid, double r, double q,
                        const Eigen::Tensor<double, 2>& sigmasGrid, double tau, grid::index::Dims dims) {
    return details::americanGreeks(spot, strikesGrid, r, q, sigmasGrid, tau, dims, true);
}

GreeksResult putGreeks(double spot, const Eigen::Tensor<double, 2>& strikesGrid, double r, double q,
                       const Eigen::Tensor<double, 2>& sigmasGrid, double tau, grid::index::Dims dims) {
    return details::americanGreeks(spot, strikesGrid, r, q, sigmasGrid, tau, dims, false);
}

} // namespace greeks::trinomial
