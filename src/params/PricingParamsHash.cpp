#include "OptionsVisualizer/params/PricingParamsHash.hpp"

#include <cstddef>
#include <functional>

#include "OptionsVisualizer/params/PricingParams.hpp"

std::size_t PricingParamsHash::operator()(
    const PricingParams& params) const noexcept {
  std::size_t h{0};

  const auto hashCombine{[&h](const std::size_t v) {
    // 64-bit version of boost::hash_combine
    h ^= v + 0x9e3779b97f4a7c15ULL + (h << 6) + (h >> 2);
  }};

  constexpr std::hash<std::int64_t> hasher{};
  hashCombine(hasher(params.nSigma_));
  hashCombine(hasher(params.nStrike_));
  hashCombine(hasher(params.spot_));
  hashCombine(hasher(params.r_));
  hashCombine(hasher(params.q_));
  hashCombine(hasher(params.sigmaLo_));
  hashCombine(hasher(params.sigmaHi_));
  hashCombine(hasher(params.strikeLo_));
  hashCombine(hasher(params.strikeHi_));
  hashCombine(hasher(params.tau_));
  return h;
}
