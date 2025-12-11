#pragma once

#include "OptionsVisualizer/math/generic_math.hpp"
#include "OptionsVisualizer/models/constants.hpp"
#include "OptionsVisualizer/payoff/Call.hpp"
#include "OptionsVisualizer/payoff/Payoff.hpp"
#include "OptionsVisualizer/payoff/Put.hpp"
#include "OptionsVisualizer/utils/compare_floats.hpp"
#include "OptionsVisualizer/utils/typing.hpp"
#include <cassert>
#include <cstddef>
#include <vector>

namespace {

// Parameter bundle for the trinomial model
template <typename T>
struct TrinomialParams {
    T dTau;
    T u;
    T d;
    T discountFactor;
    T pU;
    T pM;
    T pD;
};

// Helper: sets up multipliers, discounting and probabilities
template <typename T>
TrinomialParams<T> setupTrinomial(utils::type::ParamT<T> r, utils::type::ParamT<T> q, utils::type::ParamT<T> sigma,
                                  utils::type::ParamT<T> tau, std::size_t N) {
    // Discrete time steps
    const T dTau{tau / static_cast<int>(N)};

    // Stock price multipliers for upward and downward moves: u = e^(sigma * sqrt(3dt)); d = 1 / u
    const T u{generic::exp<T>(sigma * generic::sqrt<T>(3 * dTau))};
    const T d{1 / u};

    // Single-step discount factor: discountFactor = e^(-r * dt)
    const T discountFactor{generic::exp<T>(-r * dTau)};

    // --- Intermediate risk-neutral probability terms (see Hull - Ch.20 (444))

    // Drift factor scaling term: sqrt(dt / 12 * sigma^2)
    const T scalingTerm{generic::sqrt<T>(dTau / (12 * sigma * sigma))};

    // Log stock drift: r - q * sigma^2 / 2
    const T logStockDrift{r - q - ((sigma * sigma) / 2)};

    // Risk-neutral drift factor
    const T driftFactor{scalingTerm * logStockDrift};

    // --- Risk-neutral probabilities

    // Probability of upward price movement: p_u = sqrt(dt / 12 * sigma^2) * (r - q * sigma^2 / 2) + 1 / 6
    const T pU{driftFactor + static_cast<T>(1) / 6};

    // Probability of downward price movement: p_d = -sqrt(dt / 12 * sigma^2) * (r - q * sigma^2 / 2) + 1 / 6
    const T pD{-driftFactor + static_cast<T>(1) / 6};

    // p_m = 1 - p_u - p_d approx= 2/3
    // Middle-branch probability ensuring total probability = 1
    const T pM{1 - pU - pD};

    // Confirm p_m == 2/3
    assert(utils::qc::approxEqualAbsRel<T>(pM, static_cast<T>(2) / 3, static_cast<T>(1e-8), static_cast<T>(1e-8)));

    return TrinomialParams<T>{.dTau{dTau}, .u{u}, .d{d}, .discountFactor{discountFactor}, .pU{pU}, .pM{pM}, .pD{pD}};
}

} // namespace

namespace pricing {

/**
 * @brief Prices an American option using the trinomial options pricing model
 * @tparam T The floating-point type used (e.g., double, boost::multiprecision::cpp_dec_float_50)
 * @param payoffFun The type of the payoff function (Callable object)
 */
template <typename T>
T trinomialPrice(utils::type::ParamT<T> spot, utils::type::ParamT<T> strike, utils::type::ParamT<T> r,
                 utils::type::ParamT<T> q, utils::type::ParamT<T> sigma, utils::type::ParamT<T> tau,
                 Payoff::Payoff<T> payoffFun) {
    // Setup the function with the parameters we need
    auto [dTau, u, d, discountFactor, pU, pM, pD]{setupTrinomial<T>(r, q, sigma, tau, constants::trinomialDepth)};

    // --- Compute price using backward induction

    // Start with the lowest price
    T spotExpiration{spot * generic::pow<T, T>(d, static_cast<T>(constants::trinomialDepth))};
    std::vector<T> optionValues((constants::trinomialDepth * 2) + 1);

    // Iterate through all 2N + 1 nodes at maturity
    for (std::size_t idx{0}, twoN{2 * constants::trinomialDepth}; idx <= twoN; ++idx) {
        // At maturtity, there's only intrinsic value, no continuation value
        optionValues[idx] = payoffFun(spotExpiration, strike);
        spotExpiration *= u;
    }

    // Iterate through the remaining time steps
    for (std::size_t timeStep{constants::trinomialDepth}; timeStep-- > 0;) {
        // newOptionValues will store the option prices P_i,j
        // The number of nodes at timestep i is (2 * i + 1)
        std::vector<T> newOptionValues((2 * timeStep) + 1);
        T currentSpot{spot * generic::pow<T, T>(d, static_cast<T>(timeStep))};

        // Node index within the current time step
        for (std::size_t idx{0}, twoN{2 * timeStep}; idx <= twoN; ++idx) {
            // P_i+1,j+2 is the value from the up path
            // P_i+1,j+1 is the value from the middle path
            // P_i+1,j is the value from the down path
            const T EV{(pU * optionValues[idx + 2]) + (pM * optionValues[idx + 1]) + (pD * optionValues[idx])};

            // Calculate the continuation value
            const T continuationValue{EV * discountFactor};

            // Calculate the intrinsic value
            const T intrinsicValue{payoffFun(currentSpot, strike)};

            // The option's value is the maximum of immediate exercise (intrinsic) or holding (continuation)
            newOptionValues[idx] = generic::max<T, T>(intrinsicValue, continuationValue);

            currentSpot *= u;
        }

        optionValues = std::move(newOptionValues);
    }

    assert(optionValues.size() == 1);
    return optionValues[0];
}

} // namespace pricing