#pragma once

#include "OptionsVisualizer/lru/Params.hpp"
#include <cstddef>

struct ParamsHash {
    std::size_t operator()(const Params& params) const noexcept;
};
