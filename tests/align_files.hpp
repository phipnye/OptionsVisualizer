#pragma once

#include <filesystem>
#include <sstream>
#include <string>
#include <tuple>

#include "OptionsVisualizer/core/Enums.hpp"

// Split filename "amer_call_price" -> ("amer", "call", "price")
[[nodiscard]] inline auto splitFilename(const std::string& filename) {
  std::string nat{}, opt{}, grk{};
  std::stringstream ss{filename};
  std::getline(ss, nat, '_');
  std::getline(ss, opt, '_');
  std::getline(ss, grk);
  return std::tuple{nat, opt, grk};
}

[[nodiscard]] inline Enums::OptionType parseOption(const std::string& nat,
                                                   const std::string& opt) {
  if (nat == "amer" && opt == "call") {
    return Enums::OptionType::AmerCall;
  }
  if (nat == "amer" && opt == "put") {
    return Enums::OptionType::AmerPut;
  }
  if (nat == "euro" && opt == "call") {
    return Enums::OptionType::EuroCall;
  }
  if (nat == "euro" && opt == "put") {
    return Enums::OptionType::EuroPut;
  }

  throw std::runtime_error{"Invalid option type in filename: " + nat + "_" +
                           opt};
}

[[nodiscard]] inline Enums::GreekType parseGreek(const std::string& grk) {
  if (grk == "price") {
    return Enums::GreekType::Price;
  }
  if (grk == "delta") {
    return Enums::GreekType::Delta;
  }
  if (grk == "gamma") {
    return Enums::GreekType::Gamma;
  }
  if (grk == "vega") {
    return Enums::GreekType::Vega;
  }
  if (grk == "theta") {
    return Enums::GreekType::Theta;
  }
  if (grk == "rho") {
    return Enums::GreekType::Rho;
  }

  throw std::runtime_error("Invalid greek in filename: " + grk);
}

// Retrieve corresponding index in manager's grid result from a file name
[[nodiscard]] inline std::size_t greekIndexFromFilename(
    const std::string& filename) {
  const auto [nat, opt, grk]{splitFilename(filename)};
  const Enums::OptionType optType{parseOption(nat, opt)};
  const Enums::GreekType greekType{parseGreek(grk)};
  return Enums::idx(optType) * Enums::idx(Enums::GreekType::COUNT) +
         Enums::idx(greekType);
}
