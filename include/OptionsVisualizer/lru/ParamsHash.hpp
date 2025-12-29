#pragma once

#include <cstddef>

#include "OptionsVisualizer/lru/Params.hpp"

struct ParamsHash {
  std::size_t operator()(const Params& params) const noexcept;
};
