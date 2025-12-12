#pragma once

#include "OptionsVisualizer/math/central_difference.hpp"
#include "OptionsVisualizer/models/GreeksResult.hpp"
#include "OptionsVisualizer/models/trinomial_price.hpp"
#include "OptionsVisualizer/payoff/Call.hpp"
#include "OptionsVisualizer/payoff/Put.hpp"
#include "OptionsVisualizer/utils/typing.hpp"

namespace greeks {

/**
 * @brief Calculates the price and major greeks (delta, gamma, vega, theta) for an American option using the
 * central difference method calculations
 * @tparam T The floating-point type used (e.g., double)
 * @tparam PayoffFn The type of the payoff function (Callable object)
 * @return GreeksResult<T> A struct containing the calculated greek results
 */
template <typename T, typename PayoffFn>
GreeksResult<T> trinomialGreeks(T spot, T strike, T r, T q, T sigma, T tau, PayoffFn payoffFun) {
    // --- Calculate price, delta, and gamma
    const T epsilonSpot{0.01 * spot}; // 1% shock for delta/gamma
    const T priceBase{pricing::trinomialPrice<T, PayoffFn>(spot, strike, r, q, sigma, tau, payoffFun)};
    const T priceSpotLo{pricing::trinomialPrice<T, PayoffFn>(spot - epsilonSpot, strike, r, q, sigma, tau, payoffFun)};
    const T priceSpotHi{pricing::trinomialPrice<T, PayoffFn>(spot + epsilonSpot, strike, r, q, sigma, tau, payoffFun)};

    // Use cental difference method
    const T delta{generic::cdmFirstOrder<T>(priceSpotLo, priceSpotHi, epsilonSpot)};
    const T gamma{generic::cmdSecondOrder<T>(priceSpotLo, priceBase, priceSpotHi, epsilonSpot)};

    // --- Calculate vega
    static const T epsilonSigma{0.0001}; // 0.01% shock for vega
    const T priceSigmaLo{
        pricing::trinomialPrice<T, PayoffFn>(spot, strike, r, q, sigma - epsilonSigma, tau, payoffFun)};
    const T priceSigmaHi{
        pricing::trinomialPrice<T, PayoffFn>(spot, strike, r, q, sigma + epsilonSigma, tau, payoffFun)};
    const T vega{generic::cdmFirstOrder<T>(priceSigmaLo, priceSigmaHi, epsilonSigma)};

    // --- Calculate theta
    static const T epsilonTau{static_cast<T>(1) / 365}; // 1 day shock for theta
    const T priceTauLo{pricing::trinomialPrice<T, PayoffFn>(spot, strike, r, q, sigma, tau - epsilonTau, payoffFun)};
    const T priceTauHi{pricing::trinomialPrice<T, PayoffFn>(spot, strike, r, q, sigma, tau + epsilonTau, payoffFun)};
    const T theta{-generic::cdmFirstOrder<T>(priceTauLo, priceTauHi, epsilonTau)};
    return GreeksResult<T>{.price{priceBase}, .delta{delta}, .gamma{gamma}, .vega{vega}, .theta{theta}};
}

/**
 * @brief Calculates the price and major greeks (delta, gamma, vega, theta) for an American call using the
 * central difference method calculations
 * @tparam T The floating-point type used (e.g., double)
 * @return GreeksResult<T> A struct containing the calculated greek results
 */
template <typename T>
GreeksResult<T> trinomialCallGreeks(T spot, T strike, T r, T q, T sigma, T tau) {
    Payoff::Call<T> payoffFun{};
    return trinomialGreeks<T, Payoff::Call<T>>(spot, strike, r, q, sigma, tau, payoffFun);
}

/**
 * @brief Calculates the price and major greeks (delta, gamma, vega, theta) for an American put using the
 * central difference method calculations
 * @tparam T The floating-point type used (e.g., double)
 * @return GreeksResult<T> A struct containing the calculated greek results
 */
template <typename T>
GreeksResult<T> trinomialPutGreeks(T spot, T strike, T r, T q, T sigma, T tau) {
    Payoff::Put<T> payoffFun{};
    return trinomialGreeks<T, Payoff::Put<T>>(spot, strike, r, q, sigma, tau, payoffFun);
}

} // namespace greeks
