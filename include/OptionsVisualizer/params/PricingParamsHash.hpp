#pragma once

#include <cstddef>

#include "PricingParams.hpp"

struct PricingParamsHash {
  [[nodiscard]] std::size_t operator()(
      const PricingParams& params) const noexcept;
};
