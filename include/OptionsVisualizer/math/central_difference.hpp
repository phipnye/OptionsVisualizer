#pragma once

namespace generic {

/**
 * @brief Computes the first-order derivative of an option price with respect to a parameter using the Central
 * Difference Method (CDM)
 * @tparam T The floating-point type used (e.g., double)
 * @param priceLo The option price calculated with the parameter shocked downward (X - epsilon)
 * @param priceHi The option price calculated with the parameter shocked upward (X + epsilon)
 * @param epsilon The small perturbation amount used for the shock
 * @return T The estimated first derivative (e.g., Delta or Vega)
 */
template <typename T>
T cdmFirstOrder(T priceLo, T priceHi, T epsilon) {
    return (priceHi - priceLo) / (2 * epsilon);
}

/**
 * @brief Computes the second-order derivative of an option price with respect to a parameter using the Central
 * Difference Method (CDM)
 * @tparam T The floating-point type used (e.g., double)
 * @param priceLo The option price calculated with the parameter shocked downward (X - epsilon)
 * @param priceBase The option price calculated with the parameter at its base value (X)
 * @param priceHi The option price calculated with the parameter shocked upward (X + epsilon)
 * @param epsilon The small perturbation amount used for the shock
 * @return T The estimated second derivative (e.g., Gamma)
 */
template <typename T>
T cmdSecondOrder(T priceLo, T priceBase, T priceHi, T epsilon) {
    return (priceHi - 2 * priceBase + priceLo) / (epsilon * epsilon);
}

} // namespace generic