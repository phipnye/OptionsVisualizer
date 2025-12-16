#include "OptionsVisualizer/greeks/trinomial/details/calculate_greeks.hpp"
#include "OptionsVisualizer/greeks/trinomial/details/american_price.hpp"
#include "OptionsVisualizer/grid/Dims.hpp"
#include <OptionsVisualizer/greeks/GreeksResult.hpp>
#include <unsupported/Eigen/CXX11/Tensor>
#include <utility>

namespace greeks::trinomial::details {

GreeksResult americanGreeks(double spot, const Eigen::Tensor<double, 2>& strikesGrid, double r, double q,
                            const Eigen::Tensor<double, 2>& sigmasGrid, double tau, grid::index::Dims dims,
                            bool isCall) {
    // Define the step size for the finite difference approximation
    static constexpr double fdmStep{1e-6};

    // Compute price at current spot/sigma/tau
    Eigen::Tensor<double, 2> price{americanPrice(spot, strikesGrid, r, q, sigmasGrid, tau, dims, isCall)};

    // --- First-order derivatives (delta, vega, theta)

    // Compute delta (first derivative w.r.t spot) using Central FDM
    // Delta = (P(S + dS) - P(S - dS)) / (2 * dS)
    const double spotHi{spot + fdmStep};
    const double spotLo{spot - fdmStep};
    const Eigen::Tensor<double, 2> priceHiSpot{americanPrice(spotHi, strikesGrid, r, q, sigmasGrid, tau, dims, isCall)};
    const Eigen::Tensor<double, 2> priceLoSpot{americanPrice(spotLo, strikesGrid, r, q, sigmasGrid, tau, dims, isCall)};
    Eigen::Tensor<double, 2> delta{(priceHiSpot - priceLoSpot) / (2.0 * fdmStep)};

    // Compute vega (first derivative w.r.t sigma) using Central FDM
    // Vega = (P(sigma + dSigma) - P(sigma - dSigma)) / (2 * dSigma)
    const Eigen::Tensor<double, 2> sigmasGridHi{sigmasGrid + fdmStep};
    const Eigen::Tensor<double, 2> sigmasGridLo{sigmasGrid - fdmStep};
    const Eigen::Tensor<double, 2> priceHiSigma{
        americanPrice(spot, strikesGrid, r, q, sigmasGridHi, tau, dims, isCall)};
    const Eigen::Tensor<double, 2> priceLoSigma{
        americanPrice(spot, strikesGrid, r, q, sigmasGridLo, tau, dims, isCall)};
    Eigen::Tensor<double, 2> vega{(priceHiSigma - priceLoSigma) / (2.0 * fdmStep)};

    // Compute theta (negative first derivative w.r.t tau) using Central FDM
    // Theta = -(P(tau + dTau) - P(tau - dTau)) / (2 * dTau)
    const double tauHi{tau + fdmStep};
    const double tauLo{tau - fdmStep};
    const Eigen::Tensor<double, 2> priceHiTau{americanPrice(spot, strikesGrid, r, q, sigmasGrid, tauHi, dims, isCall)};
    const Eigen::Tensor<double, 2> priceLoTau{americanPrice(spot, strikesGrid, r, q, sigmasGrid, tauLo, dims, isCall)};
    Eigen::Tensor<double, 2> theta{-((priceHiTau - priceLoTau) / (2.0 * fdmStep))};

    // --- Second-order derivatives (gamma)

    // Compute gamma (second derivative w.r.t spot) using Central FDM
    // Gamma = (P(S + dS) - 2*P(S) + P(S - dS)) / (dS^2)
    Eigen::Tensor<double, 2> gamma{(priceHiSpot - (2.0 * price) + priceLoSpot) / (fdmStep * fdmStep)};
    return GreeksResult{std::move(price), std::move(delta), std::move(gamma), std::move(vega), std::move(theta)};
}

} // namespace greeks::trinomial::details