#pragma once

#include "OptionsVisualizer/math/central_difference.hpp"
#include "OptionsVisualizer/models/greeks_result.hpp"
#include "OptionsVisualizer/models/trinomial.hpp"
#include "OptionsVisualizer/utils/typing.hpp"

namespace greeks {

/**
 * @brief Calculates the price and major greeks (delta, gamma, vega, theta) for an American option using the
 * central difference method calculations
 * @tparam T The floating-point type used (e.g., double, boost::multiprecision::cpp_dec_float_50)
 * @return GreeksResult<T> A struct containing the calculated greek results
 */
template <typename T>
GreeksResult<T> calcTrinomialGreeks(utils::type::ParamT<T> spot, utils::type::ParamT<T> strike,
                                    utils::type::ParamT<T> r, utils::type::ParamT<T> q, utils::type::ParamT<T> sigma,
                                    utils::type::ParamT<T> tau, Payoff::Payoff<T> payoffFun) {
    // --- Calculate price, delta, and gamma
    const T epsilonSpot{0.01 * spot}; // 1% shock for delta/gamma
    const T priceBase{pricing::trinomialPrice<T>(spot, strike, r, q, sigma, tau, payoffFun)};
    const T priceSpotLo{pricing::trinomialPrice<T>(spot - epsilonSpot, strike, r, q, sigma, tau, payoffFun)};
    const T priceSpotHi{pricing::trinomialPrice<T>(spot + epsilonSpot, strike, r, q, sigma, tau, payoffFun)};

    // Use cental difference method
    const T delta{generic::cdmFirstOrder(priceSpotLo, priceSpotHi, epsilonSpot)};
    const T gamma{generic::cmdSecondOrder(priceSpotLo, priceBase, priceSpotHi, epsilonSpot)};

    // --- Calculate vega
    const T epsilonSigma{0.0001}; // 0.01% shock for vega
    const T priceSigmaLo{pricing::trinomialPrice<T>(spot, strike, r, q, sigma - epsilonSigma, tau, payoffFun)};
    const T priceSigmaHi{pricing::trinomialPrice<T>(spot, strike, r, q, sigma + epsilonSigma, tau, payoffFun)};
    const T vega{generic::cdmFirstOrder(priceSigmaLo, priceSigmaHi, epsilonSigma)};

    // --- Calculate theta
    const T epsilonTau{static_cast<T>(1) / 365}; // 1 day shock for theta
    const T priceTauLo{pricing::trinomialPrice(spot, strike, r, q, sigma, tau - epsilonTau, payoffFun)};
    const T priceTauHi{pricing::trinomialPrice(spot, strike, r, q, sigma, tau + epsilonTau, payoffFun)};
    const T theta{-generic::cdmFirstOrder(priceTauLo, priceTauHi, epsilonTau)}; // generally defined as negative partial
    return GreeksResult<T>{.price{priceBase}, .delta{delta}, .gamma{gamma}, .vega{vega}, .theta{theta}};
}

} // namespace greeks
