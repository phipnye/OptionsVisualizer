#pragma once

#include <cstddef>
#include <cstdint>

namespace Enums {

// Enum for determing type of greek
enum class GreekType : std::uint8_t {
  Price,
  Delta,
  Gamma,
  Vega,
  Theta,
  Rho,
  COUNT
};

// Enum for determining option type
enum class OptionType : std::uint8_t {
  AmerCall,
  AmerPut,
  EuroCall,
  EuroPut,
  COUNT
};

[[nodiscard]] constexpr std::size_t idx(const OptionType o) noexcept {
  return static_cast<std::size_t>(o);
}

[[nodiscard]] constexpr std::size_t idx(const GreekType g) noexcept {
  return static_cast<std::size_t>(g);
}

}  // namespace Enums
