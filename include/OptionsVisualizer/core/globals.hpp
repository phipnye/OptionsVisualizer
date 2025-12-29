#pragma once

#include <cstddef>

#include "OptionsVisualizer/core/Enums.hpp"

namespace globals {

inline constexpr std::size_t nGrids{Enums::idx(Enums::OptionType::COUNT) *
                                    Enums::idx(Enums::GreekType::COUNT)};

}  // namespace globals
