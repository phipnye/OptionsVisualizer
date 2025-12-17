#include "OptionsVisualizer/Grid/Grid.hpp"
#include <cmath>
#include <numbers>
#include <unsupported/Eigen/CXX11/Tensor>
#include <utility>

Grid::GreeksResult Grid::bsmCallGreeks() const {
    // BSM intermediate term d1
    const double sqrtTau{std::sqrt(tau_)};
    const Eigen::Tensor<double, 2> sigmaSqrtTau{sigmasGrid_ * sqrtTau};
    const Eigen::Tensor<double, 2> d1{((spot_ / strikesGrid_).log() + (tau_ * (r_ - q_ + 0.5 * sigmasGrid_.square()))) /
                                      sigmaSqrtTau};

    // BSM intermediate term d2
    const Eigen::Tensor<double, 2> d2{d1 - sigmaSqrtTau};

    // --- Standard normal CDF and PDF using erf
    const Eigen::Tensor<double, 2> cdfD1{0.5 * (1.0 + (d1 / std::sqrt(2.0)).erf())};
    const Eigen::Tensor<double, 2> cdfD2{0.5 * (1.0 + (d2 / std::sqrt(2.0)).erf())};
    const Eigen::Tensor<double, 2> pdfD1{(1.0 / std::sqrt(2.0 * std::numbers::pi)) * (-0.5 * d1.square()).exp()};

    // Constant exponential factors
    const double expQTau{std::exp(-q_ * tau_)};
    const double expRTau{std::exp(-r_ * tau_)};

    // --- Calculate results

    // price = (S * e^(-qT) * N(d1)) - (K * e^(-rT) * N(d2))
    Eigen::Tensor<double, 2> price{spot_ * expQTau * cdfD1 - strikesGrid_ * expRTau * cdfD2};

    // See Hull (ch. 18 - 398)
    // delta = e^(-qT) * N(d1)
    Eigen::Tensor<double, 2> delta{expQTau * cdfD1};

    // gamma = (N'(d1) * e^(-qT)) / (S * sigma * sqrt(T))
    Eigen::Tensor<double, 2> gamma{(pdfD1 * expQTau) / (spot_ * sigmaSqrtTau)};

    // vega = S * sqrt(T) * N'(d1) * e^(-qT)
    Eigen::Tensor<double, 2> vega{spot_ * sqrtTau * pdfD1 * expQTau};

    // theta = (-S * N'(d1) * sigma * e^(-qT) / (2 * sqrt(T))) + (q * S * N(d1) * e^(-qT)) - (r * K * e^(-rT) * N(d2))
    Eigen::Tensor<double, 2> theta{(-spot_ * pdfD1 * sigmasGrid_ * expQTau / (2 * sqrtTau)) +
                                   (q_ * spot_ * cdfD1 * expQTau) - (r_ * strikesGrid_ * expRTau * cdfD2)};

    return GreeksResult{std::move(price), std::move(delta), std::move(gamma), std::move(vega), std::move(theta)};
}

Grid::GreeksResult Grid::bsmPutGreeks() const {
    // Retrieve call counterparts
    GreeksResult callResults{bsmCallGreeks()};

    // Constant exponential factors
    const double expQTau{std::exp(-q_ * tau_)};
    const double expRTau{std::exp(-r_ * tau_)};

    // --- Calculate results

    // Put-Call Parity: P = C - S * e^(-qT) + K * e^(-rT)
    Eigen::Tensor<double, 2> price{callResults.price - spot_ * expQTau + strikesGrid_ * expRTau};

    // See Hull (ch. 18 - 398)
    // delta_put = e^(-qT) * (N(d1) - 1) = (e^(-qT) * N(d1)) - e^(-qT) = delta_call - e^(-qT)
    Eigen::Tensor<double, 2> delta{callResults.delta - expQTau};

    /*
       theta_call - theta_put = -d/dt[C - P]
                              = -d/dt[S * e^(-qT) - K * e^(-rT)]
        -> theta_put = theta_call - S * q * e^(-qT) + K * r * e^(-rT)
    */
    Eigen::Tensor<double, 2> theta{callResults.theta - (q_ * spot_ * expQTau) + (strikesGrid_ * r_ * expRTau)};

    return GreeksResult{std::move(price), std::move(delta), std::move(callResults.gamma), std::move(callResults.vega),
                        std::move(theta)};
}
