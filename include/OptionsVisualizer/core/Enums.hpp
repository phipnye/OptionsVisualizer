#pragma once

#include <cstddef>

namespace Enums {

// Enums for determing type of option
enum class GreekType : std::size_t {
  Price,
  Delta,
  Gamma,
  Vega,
  Theta,
  Rho,
  COUNT
};

enum class OptionType : std::size_t {
  AmerCall,
  AmerPut,
  EuroCall,
  EuroPut,
  COUNT
};

[[nodiscard]] constexpr std::size_t idx(const OptionType t) noexcept {
  return static_cast<std::size_t>(t);
}

[[nodiscard]] constexpr std::size_t idx(const GreekType t) noexcept {
  return static_cast<std::size_t>(t);
}

}  // namespace Enums
