#pragma once

#include "OptionsVisualizer/utils/typing.hpp"

namespace generic {

/**
 * @brief Computes the first-order derivative of an option price with respect to a parameter
 * using the Central Difference Method (CDM).
 *
 * @tparam T The floating-point type used (e.g., double, boost::multiprecision::cpp_dec_float_50).
 * @param V_lo The option price calculated with the parameter shocked downward (X - epsilon).
 * @param V_hi The option price calculated with the parameter shocked upward (X + epsilon).
 * @param epsilon The small perturbation amount used for the shock.
 * @return T The estimated first derivative (e.g., Delta or Vega).
 */
template <typename T>
T cdmFirstOrder(utils::type::ParamT<T> V_lo, utils::type::ParamT<T> V_hi, utils::type::ParamT<T> epsilon) {
    // Formula: (V(X + epsilon) - V(X - epsilon)) / (2 * epsilon)
    // This provides a more accurate approximation than forward or backward difference.
    return (V_hi - V_lo) / (2 * epsilon);
}

/**
 * @brief Computes the second-order derivative of an option price with respect to a parameter
 * using the Central Difference Method (CDM).
 *
 * This function is typically used to calculate Greeks like Gamma (d^2Price/dS^2).
 *
 * @tparam T The floating-point type used (e.g., double, boost::multiprecision::cpp_dec_float_50).
 * @param V_lo The option price calculated with the parameter shocked downward (X - epsilon).
 * @param V_base The option price calculated with the parameter at its base value (X).
 * @param V_hi The option price calculated with the parameter shocked upward (X + epsilon).
 * @param epsilon The small perturbation amount used for the shock.
 * @return T The estimated second derivative (e.g., Gamma).
 */
template <typename T>
T cmdSecondOrder(utils::type::ParamT<T> V_lo, utils::type::ParamT<T> V_base, utils::type::ParamT<T> V_hi,
                 utils::type::ParamT<T> epsilon) {
    // Formula: (V(X + epsilon) - 2*V(X) + V(X - epsilon)) / (epsilon^2)
    // This requires three price computations.
    return (V_hi - 2 * V_base + V_lo) / (epsilon * epsilon);
}

} // namespace generic