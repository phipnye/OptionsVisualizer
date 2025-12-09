#pragma once

#include "OptionsVisualizer/utils/typing.hpp"
#include <algorithm>
#include <boost/multiprecision/cpp_bin_float.hpp>
#include <boost/multiprecision/cpp_dec_float.hpp>
#include <cmath>

namespace generic {

template <typename T>
auto abs(utils::type::ParamT<T> x) {
    static_assert(utils::type::isSupportedNumber<T>,
                  "T must be a primitive float, Boost Multiprecision type, or an expression.");
    using boost::multiprecision::abs;
    using std::abs;
    return abs(x);
}

template <typename T>
auto exp(utils::type::ParamT<T> x) {
    static_assert(utils::type::isSupportedNumber<T>,
                  "T must be a primitive float, Boost Multiprecision type, or an expression.");
    using boost::multiprecision::exp;
    using std::exp;
    return exp(x);
}

template <typename T>
auto log(utils::type::ParamT<T> x) {
    static_assert(utils::type::isSupportedNumber<T>,
                  "T must be a primitive float, Boost Multiprecision type, or an expression.");
    using boost::multiprecision::log;
    using std::log;
    return log(x);
}

template <typename T1, typename T2>
auto max(utils::type::ParamT<T1> a, utils::type::ParamT<T2> b) {
    static_assert(utils::type::isSupportedNumber<T1> && utils::type::isSupportedNumber<T2>,
                  "Arguments must be primitive floats, Boost Multiprecision types, or expression templates.");
    using boost::multiprecision::max;
    using std::max;
    return max(a, b);
}

template <typename T1, typename T2>
auto min(utils::type::ParamT<T1> a, utils::type::ParamT<T2> b) {
    static_assert(utils::type::isSupportedNumber<T1> && utils::type::isSupportedNumber<T2>,
                  "Arguments must be primitive floats, Boost Multiprecision types, or expression templates.");
    using boost::multiprecision::min;
    using std::min;
    return min(a, b);
}

template <typename T1, typename T2>
auto pow(utils::type::ParamT<T1> base, utils::type::ParamT<T2> expn) {
    static_assert(utils::type::isSupportedNumber<T1> && utils::type::isSupportedNumber<T2>,
                  "Arguments must be primitive floats, Boost Multiprecision types, or expression templates.");
    using boost::multiprecision::pow;
    using std::pow;
    return pow(base, expn);
}

template <typename T>
auto sqrt(utils::type::ParamT<T> x) {
    static_assert(utils::type::isSupportedNumber<T>,
                  "T must be a primitive float, Boost Multiprecision type, or an expression.");
    using boost::multiprecision::sqrt;
    using std::sqrt;
    return sqrt(x);
}

} // namespace generic
