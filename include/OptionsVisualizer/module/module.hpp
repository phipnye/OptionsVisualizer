#pragma once

#include "OptionsVisualizer/models/black_scholes_merton.hpp"
#include "OptionsVisualizer/models/trinomial.hpp"
#include <algorithm>
#include <execution>
#include <pybind11/numpy.h>
#include <pybind11/pybind11.h>
#include <ranges>
namespace py = pybind11;

#pragma once

#include "OptionsVisualizer/models/black_scholes_merton.hpp"
#include "OptionsVisualizer/models/trinomial.hpp"
#include <algorithm>
#include <execution>
#include <pybind11/numpy.h>
#include <pybind11/pybind11.h>
#include <ranges>

namespace py = pybind11;

// Generic engine that takes a pricing function
template <typename T, typename Func>
py::array_t<T> priceGrid(T S, const py::array_t<T>& K_arr, T r, T q, const py::array_t<T>& sigma_arr, T T_exp,
                         Func priceFun) {
    static const py::ssize_t N{K_arr.size()}; // N will not change across calls
    const auto strikes{K_arr.unchecked<1>()};
    const auto volatilites{sigma_arr.unchecked<1>()};
    py::array_t<T> result(N * N);
    auto out{result.mutable_unchecked<1>()};

    std::for_each(std::execution::par_unseq, std::views::iota{py::ssize_t{0}, N},
                  [&](py::ssize_t volIdx) { // row index volatility
                      const T vol{volatilites(volIdx)};

                      for (py::ssize_t strikeIdx{0}; strikeIdx < N; ++strikeIdx) { // Column index strike prices
                          out((volIdx * N) + strikeIdx) = priceFun(S, strikes(strikeIdx), r, q, vol, T_exp);
                      }
                  });

    return result;
}

// --- Public wrappers

template <typename T>
py::array_t<T> priceAmerCalls(T S, const py::array_t<T>& K_arr, T r, T q, const py::array_t<T>& sigma_arr, T T_exp) {
    return priceGrid(S, K_arr, r, q, sigma_arr, T_exp, pricing::trinomialCall<T>);
}

template <typename T>
py::array_t<T> priceAmerPuts(T S, const py::array_t<T>& K_arr, T r, T q, const py::array_t<T>& sigma_arr, T T_exp) {
    return priceGrid(S, K_arr, r, q, sigma_arr, T_exp, pricing::trinomialPut<T>);
}

template <typename T>
py::array_t<T> priceEuroCalls(T S, const py::array_t<T>& K_arr, T r, T q, const py::array_t<T>& sigma_arr, T T_exp) {
    return priceGrid(S, K_arr, r, q, sigma_arr, T_exp, pricing::bsmCall<T>);
}

template <typename T>
py::array_t<T> priceEuroPuts(T S, const py::array_t<T>& K_arr, T r, T q, const py::array_t<T>& sigma_arr, T T_exp) {
    return priceGrid(S, K_arr, r, q, sigma_arr, T_exp, pricing::bsmPut<T>);
}
