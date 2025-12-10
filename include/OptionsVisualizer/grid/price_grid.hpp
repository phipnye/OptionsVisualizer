#pragma once

#include "OptionsVisualizer/models/black_scholes_merton.hpp"
#include "OptionsVisualizer/models/trinomial.hpp"
#include "OptionsVisualizer/utils/typing.hpp"
#include <algorithm>
#include <cassert>
#include <cstddef>
#include <execution>
#include <pybind11/numpy.h>
#include <pybind11/pybind11.h>
#include <ranges>
namespace py = pybind11;

// Generic engine that takes a pricing function
template <typename T>
py::array_t<T> priceGrids(T S, const py::array_t<T>& K_arr, T r, T q, const py::array_t<T>& sigma_arr, T T_exp) {
    static_assert(utils::type::isPrimitiveFloat<T>, "Need to update to work with boost types.");
    static constexpr py::ssize_t elsize{static_cast<py::ssize_t>(sizeof(T))}; // size of numeric type
    static const py::ssize_t N{K_arr.size()};                                 // N will not change across calls
    assert(K_arr.size() == sigma_arr.size() && "Input shapes for volatility and strike prices should be the same.");

    // Define range of values for varying model parameters
    const auto strikes{K_arr.template unchecked<1>()};
    const auto volatilites{sigma_arr.template unchecked<1>()};

    // Prepare output array
    static const auto range{std::views::iota(py::ssize_t{0}, N)};
    static const py::ssize_t shape[3]{N, N, 4};                             // Won't change across function calls
    static const py::ssize_t stride[3]{N * 4 * elsize, 4 * elsize, elsize}; // Won't change across function calls
    py::array_t<T> result{shape, stride};
    auto out{result.template mutable_unchecked<3>()};

    std::for_each(std::execution::par_unseq, range.begin(), range.end(),
                  [&](py::ssize_t volIdx) { // row index volatility
                      const T sigma{volatilites(volIdx)};

                      for (py::ssize_t strikeIdx{0}; strikeIdx < N; ++strikeIdx) { // Column index strike prices
                          const T K{strikes(strikeIdx)};
                          out(volIdx, strikeIdx, 0) = pricing::trinomialCall<T>(S, K, r, q, sigma, T_exp);
                          out(volIdx, strikeIdx, 1) = pricing::trinomialPut<T>(S, K, r, q, sigma, T_exp);
                          out(volIdx, strikeIdx, 2) = pricing::bsmCall<T>(S, K, r, q, sigma, T_exp);
                          out(volIdx, strikeIdx, 3) = pricing::bsmPut<T>(S, K, r, q, sigma, T_exp);
                      }
                  });

    return result;
}
