#pragma once

#include "OptionsVisualizer/math/generic_math.hpp"
#include "OptionsVisualizer/models/greeks_result.hpp"
#include "OptionsVisualizer/utils/typing.hpp"
#include <boost/math/distributions/normal.hpp>

namespace greeks {

/**
 * @brief Calculates the price and major greeks (delta, gamma, vega, theta) for a European Call using the
 * Black-Scholes-Merton (BSM) analytical formulas
 * @tparam T The floating-point type used (e.g., double, boost::multiprecision::cpp_dec_float_50)
 * @return GreeksResult<T> A struct containing the calculated greek results
 */
template <typename T>
GreeksResult<T> bsmCallGreeks(utils::type::ParamT<T> spot, utils::type::ParamT<T> strike, utils::type::ParamT<T> r,
                              utils::type::ParamT<T> q, utils::type::ParamT<T> sigma, utils::type::ParamT<T> tau) {
    // --- Setup

    // BSM intermediate term d1
    const T sqrtTau{generic::sqrt<T>(tau)};
    const T sigmaSqrtTau{sigma * sqrtTau};
    const T d1{(generic::log<T>(spot / strike) + (r - q + ((sigma * sigma) / 2)) * tau) / sigmaSqrtTau};

    // BSM intermediate term d2
    const T d2{d1 - sigmaSqrtTau};

    // Initialize the standard normal distribution N(0, 1)
    static const boost::math::normal_distribution<T> N01{0.0, 1.0};

    // Calculate the CDF (N(d1), N(d2)) and PDF (phi(d1)) values
    const T cdfD1{boost::math::cdf(N01, d1)};
    const T cdfD2{boost::math::cdf(N01, d2)};
    const T pdfD1{boost::math::pdf(N01, d1)};

    // Constant exponential factors
    const T expQTau{generic::exp<T>(-q * tau)};
    const T expRTau{generic::exp<T>(-r * tau)};

    // --- Calculate results

    // price = (S * e^(-qT) * N(d1)) - (K * e^(-rT) * N(d2))
    const T price{(spot * expQTau * cdfD1) - (strike * expRTau * cdfD2)};

    // delta = e^(-qT) * N(d1)
    const T delta{expQTau * cdfD1};

    // gamma = (e^(-qT) * phi(d1)) / (S * sigma * sqrt(T))
    const T gamma{(expQTau * pdfD1) / (spot * sigmaSqrtTau)};

    // vega = S * e^(-qT) * phi(d1) * sqrt(T)
    const T vega{spot * expQTau * pdfD1 * sqrtTau};

    // -theta = (S * e^(-qT) * phi(d1) * sigma) / (2 * sqrt(T)) - (r * K * e^(-rT) * N(d2)) + (q * S * e^(-qT) * N(d1))
    const T theta{-(((-spot * expQTau * pdfD1 * sigma) / (2 * sqrtTau)) - (r * strike * expRTau * cdfD2) +
                    (q * spot * expQTau * cdfD1))}; // generally defined as negative partial

    return GreeksResult<T>{.price{price}, .delta{delta}, .gamma{gamma}, .vega{vega}, .theta{theta}};
}

/**
 * @brief Calculates the price and major greeks (delta, gamma, vega, theta) for a European Put using the
 * Black-Scholes-Merton (BSM) analytical formulas
 * @tparam T The floating-point type used (e.g., double, boost::multiprecision::cpp_dec_float_50)
 * @return GreeksResult<T> A struct containing the calculated greek results
 */
template <typename T>
GreeksResult<T> bsmPutGreeks(utils::type::ParamT<T> spot, utils::type::ParamT<T> strike, utils::type::ParamT<T> r,
                             utils::type::ParamT<T> q, utils::type::ParamT<T> sigma, utils::type::ParamT<T> tau) {
    // --- Setup

    // Retrieve call counterparts
    const auto [callPrice, callDelta, callGamma, callVega, callTheta]{bsmCallGreeks<T>(spot, strike, r, q, sigma, tau)};

    // Constant exponential factors
    const T expQTau{generic::exp<T>(-q * tau)};
    const T expRTau{generic::exp<T>(-r * tau)};

    // --- Calculate results

    // Put-Call Parity: P = C - S * e^(-qT) + K * e^(-rT)
    const T price{callPrice - (spot * expQTau) + (strike * expRTau)};

    // delta_put = delta_call - e^(-qT)
    const T delta{callDelta - expQTau};

    // -theta = -theta_call - r * K * e^(-rT) + q * S * e^(-qT)
    const T theta{callTheta + (r * strike * expRTau) - (q * spot * expQTau)};

    return GreeksResult<T>{.price{price}, .delta{delta}, .gamma{callGamma}, .vega{callVega}, .theta{theta}};
}

} // namespace greeks