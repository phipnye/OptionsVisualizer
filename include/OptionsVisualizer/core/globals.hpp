#pragma once

#include "OptionsVisualizer/core/Enums.hpp"
#include <cstddef>

namespace globals {

inline constexpr std::size_t nGrids{Enums::idx(Enums::OptionType::COUNT) * Enums::idx(Enums::GreekType::COUNT)};

} // namespace globals
