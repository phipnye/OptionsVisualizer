#pragma once

#include "OptionsVisualizer/math/generic_math.hpp"
#include "OptionsVisualizer/utils/typing.hpp"

namespace utils::qc {

/**
 * @brief Return true if the difference between a and b is within epsilon percent of the larger of a and b.
 * @tparam T The numeric type (e.g., double, boost::multiprecision::cpp_dec_float_50).
 */
template <typename T>
bool approxEqualRel(utils::type::ParamT<T> a, utils::type::ParamT<T> b, utils::type::ParamT<T> relEpsilon) {
    return (generic::abs<T>(a - b) <= (generic::max<T, T>(generic::abs<T>(a), generic::abs<T>(b)) * relEpsilon));
}

/**
 * @brief Return true if the difference between a and b is less than or equal to absEpsilon, or within relEpsilon
 * percent of the larger of a and b
 * @tparam T The numeric type (e.g., double, boost::multiprecision::cpp_dec_float_50).
 */
template <typename T>
bool approxEqualAbsRel(utils::type::ParamT<T> a, utils::type::ParamT<T> b, utils::type::ParamT<T> absEpsilon,
                       utils::type::ParamT<T> relEpsilon) {
    // Check if the numbers are really close (for when comparing numbers near zero)
    if (generic::abs<T>(a - b) <= absEpsilon) {
        return true;
    }

    // Otherwise fall back to Knuth's algorithm
    return approxEqualRel<T>(a, b, relEpsilon);
}

} // namespace utils::qc
