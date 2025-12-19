#pragma once

#include <cstddef>

namespace Enums {

// Enums for determing type of option
enum class GreekType : std::size_t { Price = 0, Delta = 1, Gamma = 2, Vega = 3, Theta = 4, Rho = 5, COUNT = 6 };
enum class OptionType : std::size_t { AmerCall = 0, AmerPut = 1, EuroCall = 2, EuroPut = 3, COUNT = 4 };

static constexpr std::size_t idx(OptionType t) noexcept {
    return static_cast<std::size_t>(t);
}

static constexpr std::size_t idx(GreekType t) noexcept {
    return static_cast<std::size_t>(t);
}

} // namespace Enums
