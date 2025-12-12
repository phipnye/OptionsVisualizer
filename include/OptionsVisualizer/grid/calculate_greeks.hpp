#pragma once

#include "OptionsVisualizer/grid/index.hpp"
#include "OptionsVisualizer/models/GreeksResult.hpp"
#include "OptionsVisualizer/models/bsm_greeks.hpp"
#include "OptionsVisualizer/models/trinomial_greeks.hpp"
#include "OptionsVisualizer/utils/typing.hpp"
#include <algorithm>
#include <cassert>
#include <cstddef>
#include <execution>
#include <pybind11/numpy.h>
#include <pybind11/pybind11.h>
#include <ranges>
namespace py = pybind11;

namespace grid {

template <typename T>
py::array_t<T> calculateGreeksGrid(T spot, py::array_t<T, py::array::c_style> strikesArr, T r, T q,
                                   py::array_t<T, py::array::c_style> sigmaArr, T tau) {
    static_assert(utils::type::isPrimitiveFloat<T>, "Numpy arrays cannot store boost::multiprecision types.");
    assert(strikesArr.ndim() == 1 && sigmaArr.ndim() == 1 && "Unexpected number of dimensions.");
    const std::size_t N{static_cast<std::size_t>(strikesArr.shape(0))};
    
    // Get direct, non-owning pointers to the raw data
    const T* const strikesPtr{strikesArr.data()};
    const T* const sigmasPtr{sigmaArr.data()};
    
    // Release gill since we've accessed array pointers and no longer need access to python objects
    py::gil_scoped_release noGil{};

    // Prepare output array
    std::vector<T> result(N * N * 4 * 5); // volatility x strike x option type x greek type
    const auto range{std::views::iota(std::size_t{0}, N)};

    std::for_each(
        std::execution::par_unseq, range.begin(), range.end(),
        [&](std::size_t sigmaIdx) { // row index volatility
            const T sigma{sigmasPtr[sigmaIdx]};

            for (std::size_t strikeIdx{0}; strikeIdx < N; ++strikeIdx) { // Column index strike prices
                const T strike{strikesPtr[strikeIdx]};
                
                // Calculate greek results
                const greeks::GreeksResult<T> amerCall{greeks::trinomialCallGreeks<T>(spot, strike, r, q, sigma, tau)};
                const greeks::GreeksResult<T> amerPut{greeks::trinomialPutGreeks<T>(spot, strike, r, q, sigma, tau)};
                const greeks::GreeksResult<T> euroCall{greeks::bsmCallGreeks<T>(spot, strike, r, q, sigma, tau)};
                const greeks::GreeksResult<T> euroPut{greeks::bsmPutGreeks<T>(spot, strike, r, q, sigma, tau)};
                
                // Store results in a vector
                index::writeGreeksToResult(result, N, sigmaIdx, strikeIdx, index::OptionType::AmerCall, amerCall);
                index::writeGreeksToResult(result, N, sigmaIdx, strikeIdx, index::OptionType::AmerPut, amerPut);
                index::writeGreeksToResult(result, N, sigmaIdx, strikeIdx, index::OptionType::EuroCall, euroCall);
                index::writeGreeksToResult(result, N, sigmaIdx, strikeIdx, index::OptionType::EuroPut, euroPut);
            }
        });

    py::gil_scoped_acquire acquireGil{};

    // https://github.com/pybind/pybind11/issues/1042#issuecomment-325941022
    // Link the lifetime of the underlying data to the lifetime of the python object (numpy array)
    std::vector<T>* resultPtr{new std::vector<T>(std::move(result))};

    // Capsule determines how to delete this object later
    auto capsule{py::capsule{resultPtr, [](void* p) { delete reinterpret_cast<std::vector<T>*>(p); }}};
    return py::array_t<T>(resultPtr->size(), resultPtr->data(), capsule);
}

} // namespace grid
