#pragma once

#include <cmath>

namespace utils::qc {

/**
 * @brief Return true if the difference between a and b is within epsilon percent of the larger of a and b
 * @tparam T The floating-point type used (e.g., double)
 */
template <typename T>
bool approxEqualRel(T a, T b, T relEpsilon) {
    return (std::abs(a - b) <= (std::max(std::abs(a), std::abs(b)) * relEpsilon));
}

/**
 * @brief Return true if the difference between a and b is less than or equal to absEpsilon, or within relEpsilon
 * percent of the larger of a and b
 * @tparam T The floating-point type used (e.g., double)
 */
template <typename T>
bool approxEqualAbsRel(T a, T b, T absEpsilon, T relEpsilon) {
    // Check if the numbers are really close (for when comparing numbers near zero)
    if (std::abs(a - b) <= absEpsilon) {
        return true;
    }

    // Otherwise fall back to Knuth's algorithm
    return approxEqualRel<T>(a, b, relEpsilon);
}

} // namespace utils::qc
