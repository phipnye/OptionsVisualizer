#pragma once

#include "OptionsVisualizer/math/generic_math.hpp"
#include "OptionsVisualizer/utils/typing.hpp"
#include <boost/math/distributions/normal.hpp>

namespace pricing {

/**
 * @brief Prices a European Call option using the Black-Scholes-Merton (BSM) model.
 * @tparam T The numeric type (e.g., double, boost::multiprecision::cpp_dec_float_50).
 */
template <typename T>
T bsmCall(utils::type::ParamT<T> S, utils::type::ParamT<T> K, utils::type::ParamT<T> r, utils::type::ParamT<T> q,
          utils::type::ParamT<T> sigma, utils::type::ParamT<T> T_exp) {
    const T sqrtT{generic::sqrt<T>(T_exp)};
    const T sigmaSqrtT{sigma * sqrtT};

    // BSM intermediate term d1: captures risk-neutral drift adjusted for dividends
    const T d1{(generic::log<T>(S / K) + (r - q + ((sigma * sigma) / 2)) * T_exp) / sigmaSqrtT};

    // BSM intermediate term d2: d2 = d1 - sigma * sqrt(T)
    const T d2{d1 - sigmaSqrtT};

    // Initialize the standard normal distribution N(0, 1)
    static const boost::math::normal_distribution<T> N01{0.0, 1.0};

    // Calculate the cumulative distribution function (CDF) values: N(d1) and N(d2)
    const T Nd1{boost::math::cdf(N01, d1)};
    const T Nd2{boost::math::cdf(N01, d2)};

    // Final BSM Call Price Formula: C = S * e^(-qT) * N(d1) - K * e^(-rT) * N(d2)
    return (S * generic::exp<T>(-q * T_exp) * Nd1) - (K * generic::exp<T>(-r * T_exp) * Nd2);
}

/**
 * @brief Prices a European Put option using the Black-Scholes-Merton Call price and Put-Call Parity.
 * @tparam T The numeric type.
 */
template <typename T>
T bsmPut(utils::type::ParamT<T> S, utils::type::ParamT<T> K, utils::type::ParamT<T> r, utils::type::ParamT<T> q,
         utils::type::ParamT<T> sigma, utils::type::ParamT<T> T_exp) {
    // Put-Call Parity: P = C - S * e^(-qT) + K * e^(-rT)
    const T callPrice{bsmCall<T>(S, K, r, q, sigma, T_exp)};
    return callPrice - (S * generic::exp<T>(-q * T_exp)) + (K * generic::exp<T>(-r * T_exp));
}

} // namespace pricing