#pragma once

#include <boost/multiprecision/cpp_bin_float.hpp>
#include <boost/multiprecision/cpp_dec_float.hpp>
#include <type_traits>

namespace utils::type {
template <typename T>
constexpr bool isPrimitiveFloat = std::is_floating_point_v<T>;

template <typename T>
constexpr bool isConcreteMultiprecision = boost::multiprecision::is_number<T>::value;

template <typename T>
constexpr bool isExpressionTemplate = boost::multiprecision::is_number_expression<T>::value;

template <typename T>
constexpr bool isSupportedNumber = isPrimitiveFloat<T> || isConcreteMultiprecision<T> || isExpressionTemplate<T>;

template <typename T>
using ParamT = std::conditional_t<isPrimitiveFloat<T>,
                                  T,       // pass by value for doubles/floats
                                  const T& // pass by const-ref for multiprecision + expressions
                                  >;
} // namespace utils::type
