#pragma once

#include "OptionsVisualizer/models/GreeksResult.hpp"
#include <boost/math/distributions/normal.hpp>
#include <cmath>

namespace greeks {

/**
 * @brief Calculates the price and major greeks (delta, gamma, vega, theta) for a European Call using the
 * Black-Scholes-Merton (BSM) analytical formulas
 * @tparam T The floating-point type used (e.g., double)
 * @return GreeksResult<T> A struct containing the calculated greek results
 */
template <typename T>
GreeksResult<T> bsmCallGreeks(T spot, T strike, T r, T q, T sigma, T tau) {
    // --- Setup

    // BSM intermediate term d1
    const T sqrtTau{std::sqrt(tau)};
    const T sigmaSqrtTau{sigma * sqrtTau};
    const T d1{(std::log(spot / strike) + (r - q + ((sigma * sigma) / 2)) * tau) / sigmaSqrtTau};

    // BSM intermediate term d2
    const T d2{d1 - sigmaSqrtTau};

    // Initialize the standard normal distribution N(0, 1)
    static const boost::math::normal_distribution<T> N01{0.0, 1.0};

    // Calculate the CDF (N(d1), N(d2)) and PDF (N'(d1)) values
    const T cdfD1{boost::math::cdf(N01, d1)};
    const T cdfD2{boost::math::cdf(N01, d2)};
    const T pdfD1{boost::math::pdf(N01, d1)};

    // Constant exponential factors
    const T expQTau{std::exp(-q * tau)};
    const T expRTau{std::exp(-r * tau)};

    // --- Calculate results

    // price = (S * e^(-qT) * N(d1)) - (K * e^(-rT) * N(d2))
    const T price{(spot * expQTau * cdfD1) - (strike * expRTau * cdfD2)};

    // See Hull (ch. 18 - 398)
    // delta = e^(-qT) * N(d1)
    const T delta{expQTau * cdfD1};

    // gamma = (N'(d1) * e^(-qT)) / (S * sigma * sqrt(T))
    const T gamma{(pdfD1 * expQTau) / (spot * sigmaSqrtTau)};

    // vega = S * sqrt(T) * N'(d1) * e^(-qT)
    const T vega{spot * sqrtTau * pdfD1 * expQTau};

    // theta = (-S * N'(d1) * sigma * e^(-qT) / (2 * sqrt(T))) + (q * S * N(d1) * e^(-qT)) - (r * K * e^(-rT) * N(d2))
    const T theta{(-spot * pdfD1 * sigma * expQTau / (2 * sqrtTau)) + (q * spot * cdfD1 * expQTau) -
                  (r * strike * expRTau * cdfD2)};

    return GreeksResult<T>{.price{price}, .delta{delta}, .gamma{gamma}, .vega{vega}, .theta{theta}};
}

/**
 * @brief Calculates the price and major greeks (delta, gamma, vega, theta) for a European Put using the
 * Black-Scholes-Merton (BSM) analytical formulas
 * @tparam T The floating-point type used (e.g., double)
 * @return GreeksResult<T> A struct containing the calculated greek results
 */
template <typename T>
GreeksResult<T> bsmPutGreeks(T spot, T strike, T r, T q, T sigma, T tau) {
    // --- Setup

    // Retrieve call counterparts
    const auto [callPrice, callDelta, callGamma, callVega, callTheta]{bsmCallGreeks<T>(spot, strike, r, q, sigma, tau)};

    // Constant exponential factors
    const T expQTau{std::exp(-q * tau)};
    const T expRTau{std::exp(-r * tau)};

    // --- Calculate results

    // Put-Call Parity: P = C - S * e^(-qT) + K * e^(-rT)
    const T price{callPrice - (spot * expQTau) + (strike * expRTau)};

    // See Hull (ch. 18 - 398)
    // delta_put = e^(-qT) * (N(d1) - 1) = (e^(-qT) * N(d1)) - e^(-qT) = delta_call - e^(-qT)
    const T delta{callDelta - expQTau};

    /*
       theta_call - theta_put = -d/dt[C - P]
                              = -d/dt[S * e^(-qT) - K * e^(-rT)]
                              = S * q * e^(-qT) - K * r * e^(-rT)

        -> theta_put = theta_call - S * q * e^(-qT) + K * r * e^(-rT)
    */
    const T theta{callTheta - (q * spot * expQTau) + (r * strike * expRTau)};

    return GreeksResult<T>{.price{price}, .delta{delta}, .gamma{callGamma}, .vega{callVega}, .theta{theta}};
}

} // namespace greeks
