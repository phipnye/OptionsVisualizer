#include "OptionsVisualizer/pricing/GreeksResult.hpp"
#include "OptionsVisualizer/pricing/PricingSurface.hpp"
#include <Eigen/Dense>
#include <cmath>
#include <numbers>
#include <unsupported/Eigen/SpecialFunctions> // error function
#include <utility>

GreeksResult PricingSurface::bsmCallGreeks() const {
    // BSM intermediate term d1 = (log(S / K) + ((r - q - simga^2 / 2) * T)) / sigma * sqrt(T)
    const double sqrtTau{std::sqrt(tau_)};
    const auto sigmaSqrtTau{sigmasGrid_ * sqrtTau};
    const auto d1{((spot_ / strikesGrid_.array()).log() + ((r_ - q_ + 0.5 * sigmasGrid_.array().square()) * tau_)) /
                  sigmaSqrtTau.array()};

    // BSM intermediate term d2 = d1 - sigma * sqrt(T)
    const auto d2{d1 - sigmaSqrtTau.array()};

    // --- Standard normal CDF and PDF using error function
    const auto cdfD1{0.5 * (1.0 + (d1.array() / std::sqrt(2.0)).erf())};
    const auto cdfD2{0.5 * (1.0 + (d2.array() / std::sqrt(2.0)).erf())};
    const auto pdfD1{(1.0 / std::sqrt(2.0 * std::numbers::pi)) * (-0.5 * d1.array().square()).exp()};

    // Constant exponential factors
    const double expQTau{std::exp(-q_ * tau_)};
    const double expRTau{std::exp(-r_ * tau_)};

    // --- Calculate results

    // price = (S * e^(-qT) * N(d1)) - (K * e^(-rT) * N(d2))
    Eigen::MatrixXd price{spot_ * expQTau * cdfD1.matrix() - (strikesGrid_.array() * expRTau * cdfD2.array()).matrix()};

    // See Hull (ch. 18 - 398)
    // delta = e^(-qT) * N(d1)
    Eigen::MatrixXd delta{expQTau * cdfD1.matrix()};

    // gamma = (N'(d1) * e^(-qT)) / (S * sigma * sqrt(T))
    Eigen::MatrixXd gamma{(pdfD1.array() * expQTau) / (spot_ * sigmaSqrtTau.array())};

    // vega = S * sqrt(T) * N'(d1) * e^(-qT)
    Eigen::MatrixXd vega{(spot_ * sqrtTau * pdfD1.array() * expQTau).matrix()};

    // theta = (-S * N'(d1) * sigma * e^(-qT) / (2 * sqrt(T))) + (q * S * N(d1) * e^(-qT)) - (r * K * e^(-rT) * N(d2))
    Eigen::MatrixXd theta{(-spot_ * pdfD1.array() * sigmasGrid_.array() * expQTau / (2.0 * sqrtTau)) +
                          (q_ * spot_ * cdfD1.array() * expQTau) -
                          (r_ * strikesGrid_.array() * expRTau * cdfD2.array())};

    // rho = K * T * e^(-rT) * N(d2)
    Eigen::MatrixXd rho{(strikesGrid_.array() * tau_ * expRTau * cdfD2.array()).matrix()};

    return GreeksResult{std::move(price), std::move(delta), std::move(gamma),
                        std::move(vega),  std::move(theta), std::move(rho)};
}

GreeksResult PricingSurface::bsmPutGreeks(const GreeksResult& callResults) const {
    // Constant exponential factors
    const double expQTau{std::exp(-q_ * tau_)};
    const double expRTau{std::exp(-r_ * tau_)};

    // --- Calculate results

    // Put-Call Parity: P = C - S * e^(-qT) + K * e^(-rT)
    Eigen::MatrixXd price{callResults.price.array() - spot_ * expQTau + strikesGrid_.array() * expRTau};

    // See Hull (ch. 18 - 398)
    // delta_put = e^(-qT) * (N(d1) - 1) = (e^(-qT) * N(d1)) - e^(-qT) = delta_call - e^(-qT)
    Eigen::MatrixXd delta{callResults.delta.array() - expQTau};

    /*
       theta_call - theta_put = -d/dt[C - P]
                              = -d/dt[S * e^(-qT) - K * e^(-rT)]
        -> theta_put = theta_call - S * q * e^(-qT) + K * r * e^(-rT)
    */
    Eigen::MatrixXd theta{callResults.theta.array() - (spot_ * q_ * expQTau) + (strikesGrid_.array() * r_ * expRTau)};

    /*
        rho_call - rho_put = d/dr[C - P]
                           = d/dr[S - K * e^(-rT)]
                           = K * T * e^(-rT)
        -> rho_put = rho_call - K * T * e^(-rT)
    */
    Eigen::MatrixXd rho{callResults.rho.array() - (strikesGrid_.array() * tau_ * expRTau)};

    return GreeksResult{
        std::move(price), std::move(delta), Eigen::MatrixXd{callResults.gamma}, Eigen::MatrixXd{callResults.vega},
        std::move(theta), std::move(rho)};
}
