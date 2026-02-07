#pragma once

#include <cstddef>

#include "OptionsVisualizer/core/Enums.hpp"

namespace globals {

// Total number of grids computed for a single set of parameters (number of
// option types x number of greek types)
inline constexpr std::size_t nGrids{Enums::idx(Enums::OptionType::COUNT) *
                                    Enums::idx(Enums::GreekType::COUNT)};

}  // namespace globals
