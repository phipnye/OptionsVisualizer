#pragma once

#include "OptionsVisualizer/models/GreeksResult.hpp"
#include <pybind11/pybind11.h>
namespace py = pybind11;

namespace grid::index {

enum class OptionType : std::size_t { AmerCall = 0, AmerPut = 1, EuroCall = 2, EuroPut = 3 };
enum class GreekType : std::size_t { Price = 0, Delta = 1, Gamma = 2, Vega = 3, Theta = 4 };

constexpr std::size_t idx(OptionType t) noexcept {
    return static_cast<std::size_t>(t);
}

constexpr std::size_t idx(GreekType g) noexcept {
    return static_cast<std::size_t>(g);
}

/**
 * @brief Writes GreeksResult into a flat contiguous buffer representing a 4D grid
 * (volatilty x strike x option type x greek type)
 *
 * Layout (row-major):
 *     index = (((sigmaIdx * N + strikeIdx) * 4) + optionTypeIdx) * 5 + greekType
 *
 * @tparam T float/double
 * @param result Flat contiguous buffer of size N*N*4*5
 * @param N Grid resolution (same length for strike and sigma)
 * @param sigmaIdx Index in volatility dimension
 * @param strikeIdx Index in strike dimension
 * @param optionTypeIdx Enum: AmerCall, AmerPut, EuroCall, EuroPut
 * @param greeks GreeksResult<T> containing price, delta, gamma, vega, theta
 */
template <typename T>
void writeGreeksToResult(std::vector<T>& result, std::size_t N, std::size_t sigmaIdx, std::size_t strikeIdx,
                         OptionType optionTypeIdx, const greeks::GreeksResult<T>& greeks) {
    static constexpr std::size_t PRICE{idx(GreekType::Price)};
    static constexpr std::size_t DELTA{idx(GreekType::Delta)};
    static constexpr std::size_t GAMMA{idx(GreekType::Gamma)};
    static constexpr std::size_t VEGA{idx(GreekType::Vega)};
    static constexpr std::size_t THETA{idx(GreekType::Theta)};

    // Compute the base index for the given sigma, strike, option type combination
    const std::size_t base{(((sigmaIdx * N + strikeIdx) * 4) + idx(optionTypeIdx)) * 5};
    result[base + PRICE] = greeks.price;
    result[base + DELTA] = greeks.delta;
    result[base + GAMMA] = greeks.gamma;
    result[base + VEGA] = greeks.vega;
    result[base + THETA] = greeks.theta;
}

} // namespace grid::index
