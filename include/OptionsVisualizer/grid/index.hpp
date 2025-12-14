#pragma once

#include "OptionsVisualizer/greeks/GreeksResult.hpp"
#include <cstdint>
#include <torch/torch.h>

namespace grid::index {

enum class GreekType : std::int64_t { Price, Delta, Gamma, Vega, Theta };
enum class OptionType : std::int64_t { AmerCall, AmerPut, EuroCall, EuroPut };

constexpr std::int64_t idx(OptionType t) noexcept;
constexpr std::int64_t idx(GreekType t) noexcept;
void writeOptionGreeks(torch::Tensor& output, index::OptionType option, const greeks::GreeksResult& g);

} // namespace grid::index
