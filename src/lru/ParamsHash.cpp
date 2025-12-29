#include "OptionsVisualizer/lru/ParamsHash.hpp"

#include <cstddef>
#include <functional>

#include "OptionsVisualizer/lru/Params.hpp"

std::size_t ParamsHash::operator()(const Params& params) const noexcept {
  std::size_t h{0};

  const auto hashCombine{[&h](const std::size_t v) {
    // 64-bit version of boost::hash_combine
    h ^= v + 0x9e3779b97f4a7c15ULL + (h << 6) + (h >> 2);
  }};

  hashCombine(std::hash<std::int64_t>{}(params.nSigma));
  hashCombine(std::hash<std::int64_t>{}(params.nStrike));
  hashCombine(std::hash<std::int64_t>{}(params.spot));
  hashCombine(std::hash<std::int64_t>{}(params.r));
  hashCombine(std::hash<std::int64_t>{}(params.q));
  hashCombine(std::hash<std::int64_t>{}(params.sigmaLo));
  hashCombine(std::hash<std::int64_t>{}(params.sigmaHi));
  hashCombine(std::hash<std::int64_t>{}(params.strikeLo));
  hashCombine(std::hash<std::int64_t>{}(params.strikeHi));
  hashCombine(std::hash<std::int64_t>{}(params.tau));
  return h;
}
