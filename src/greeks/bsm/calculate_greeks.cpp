#include "OptionsVisualizer/greeks/bsm/calculate_greeks.hpp"
#include "OptionsVisualizer/greeks/GreeksResult.hpp"
#include <Eigen/Dense>
#include <cmath>
#include <unsupported/Eigen/CXX11/Tensor>
#include <utility>

namespace greeks::bsm {

GreeksResult callGreeks(double spot, const Eigen::Tensor<double, 2>& strikesGrid, double r, double q,
                        const Eigen::Tensor<double, 2>& sigmasGrid, double tau) {
    // --- Setup

    // BSM intermediate term d1
    const double sqrtTau{std::sqrt(tau)};
    const Eigen::Tensor<double, 2> sigmaSqrtTau{sigmasGrid * sqrtTau};
    const Eigen::Tensor<double, 2> d1{((spot / strikesGrid).log() + (tau * (r - q + 0.5 * sigmasGrid.square()))) /
                                      sigmaSqrtTau};

    // BSM intermediate term d2
    const Eigen::Tensor<double, 2> d2{d1 - sigmaSqrtTau};

    // --- Standard normal CDF and PDF using erf
    const Eigen::Tensor<double, 2> cdfD1{0.5 * (1.0 + (d1 / std::sqrt(2.0)).erf())};
    const Eigen::Tensor<double, 2> cdfD2{0.5 * (1.0 + (d2 / std::sqrt(2.0)).erf())};
    const Eigen::Tensor<double, 2> pdfD1{(1.0 / std::sqrt(2.0 * M_PI)) * (-0.5 * d1.square()).exp()};

    // Constant exponential factors
    const double expQTau{std::exp(-q * tau)};
    const double expRTau{std::exp(-r * tau)};

    // --- Calculate results

    // price = (S * e^(-qT) * N(d1)) - (K * e^(-rT) * N(d2))
    Eigen::Tensor<double, 2> price{spot * expQTau * cdfD1 - strikesGrid * expRTau * cdfD2};

    // See Hull (ch. 18 - 398)
    // delta = e^(-qT) * N(d1)
    Eigen::Tensor<double, 2> delta{expQTau * cdfD1};

    // gamma = (N'(d1) * e^(-qT)) / (S * sigma * sqrt(T))
    Eigen::Tensor<double, 2> gamma{(pdfD1 * expQTau) / (spot * sigmaSqrtTau)};

    // vega = S * sqrt(T) * N'(d1) * e^(-qT)
    Eigen::Tensor<double, 2> vega{spot * sqrtTau * pdfD1 * expQTau};

    // theta = (-S * N'(d1) * sigma * e^(-qT) / (2 * sqrt(T))) + (q * S * N(d1) * e^(-qT)) - (r * K * e^(-rT) * N(d2))
    Eigen::Tensor<double, 2> theta{(-spot * pdfD1 * sigmasGrid * expQTau / (2 * sqrtTau)) +
                                   (q * spot * cdfD1 * expQTau) - (r * strikesGrid * expRTau * cdfD2)};

    return GreeksResult{std::move(price), std::move(delta), std::move(gamma), std::move(vega), std::move(theta)};
}

GreeksResult putGreeks(double spot, const Eigen::Tensor<double, 2>& strikesGrid, double r, double q,
                       const Eigen::Tensor<double, 2>& sigmasGrid, double tau) {
    // --- Setup

    // Retrieve call counterparts
    GreeksResult callResults{callGreeks(spot, strikesGrid, r, q, sigmasGrid, tau)};

    // Constant exponential factors
    const double expQTau{std::exp(-q * tau)};
    const double expRTau{std::exp(-r * tau)};

    // --- Calculate results

    // Put-Call Parity: P = C - S * e^(-qT) + K * e^(-rT)
    Eigen::Tensor<double, 2> price{callResults.price - spot * expQTau + strikesGrid * expRTau};

    // See Hull (ch. 18 - 398)
    // delta_put = e^(-qT) * (N(d1) - 1) = (e^(-qT) * N(d1)) - e^(-qT) = delta_call - e^(-qT)
    Eigen::Tensor<double, 2> delta{callResults.delta - expQTau};

    /*
       theta_call - theta_put = -d/dt[C - P]
                              = -d/dt[S * e^(-qT) - K * e^(-rT)]
        -> theta_put = theta_call - S * q * e^(-qT) + K * r * e^(-rT)
    */
    Eigen::Tensor<double, 2> theta{callResults.theta - (q * spot * expQTau) + (strikesGrid * r * expRTau)};

    return GreeksResult{std::move(price), std::move(delta), std::move(callResults.gamma), std::move(callResults.vega),
                        std::move(theta)};
}

} // namespace greeks::bsm
