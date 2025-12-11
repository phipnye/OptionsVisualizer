#pragma once

#include "OptionsVisualizer/greeks/greeks_result.hpp"
#include "OptionsVisualizer/math/central_difference.hpp"
#include "OptionsVisualizer/models/trinomial.hpp"
#include "OptionsVisualizer/utils/typing.hpp"

template <typename T>
GreeksResult<T> calcTrinomialGreeks(utils::type::ParamT<T> S, utils::type::ParamT<T> K, utils::type::ParamT<T> r,
                                    utils::type::ParamT<T> q, utils::type::ParamT<T> sigma,
                                    utils::type::ParamT<T> T_exp, Payoff::Payoff<T> payoffFun) {
    // --- Calculate price, delta, and gamma
    const T epsilon_S{0.01 * S}; // 1% shock for delta/gamma
    const T V_base{pricing::trinomialPrice<T>(S, K, r, q, sigma, T_exp, payoffFun)};
    const T V_S_lo{
        pricing::trinomialPrice<T>(S - epsilon_S, K, r, q, sigma, T_exp, payoffFun)}; // shock down value(S - eps)
    const T V_S_hi{
        pricing::trinomialPrice<T>(S + epsilon_S, K, r, q, sigma, T_exp, payoffFun)}; // shock up value(S + eps)

    // Calculate delta and gamma using cental difference method
    const T delta{generic::cdmFirstOrder(V_S_lo, V_S_hi, epsilon_S)};
    const T gamma{generic::cmdSecondOrder(V_S_lo, V_base, V_S_hi, epsilon_S)};

    // --- Calculate vega
    const T epsilon_sigma{0.0001}; // 0.01% shock for vega
    const T V_sigma_lo{pricing::trinomialPrice<T>(S, K, r, q, sigma - epsilon_sigma, T_exp, payoffFun)};
    const T V_sigma_hi{pricing::trinomialPrice<T>(S, K, r, q, sigma + epsilon_sigma, T_exp, payoffFun)};
    const T vega{generic::cdmFirstOrder(V_sigma_lo, V_sigma_hi, epsilon_sigma)};

    // --- Calculate theta
    const T epsilon_T{static_cast<T>(1) / 365}; // 1 day shock for theta
    const T V_T_lo{pricing::trinomialPrice(S, K, r, q, sigma, T_exp - epsilon_T, payoffFun)};
    const T V_T_hi{pricing::trinomialPrice(S, K, r, q, sigma, T_exp + epsilon_T, payoffFun)};
    const T theta{-generic::cdmFirstOrder(V_T_lo, V_T_hi, epsilon_T)}; // generally defined as negative partial

    return GreeksResult<T>{
        .price = V_base,
        .delta = delta,
        .gamma = gamma,
        .vega = vega,
        .theta = theta
    };
}