#include "OptionsVisualizer/pricing/PricingParams.hpp"

#include <Eigen/Dense>
#include <array>
#include <cstddef>
#include <cstdint>
#include <functional>

PricingParams::PricingParams(const Eigen::Index nSigma,
                             const Eigen::Index nStrike, const double spot,
                             const double r, const double q,
                             const double sigmaLo, const double sigmaHi,
                             const double strikeLo, const double strikeHi,
                             const double tau)
    : data_{static_cast<std::int64_t>(nSigma),
            static_cast<std::int64_t>(nStrike),
            quantize(spot),
            quantize(r),
            quantize(q),
            quantize(sigmaLo),
            quantize(sigmaHi),
            quantize(strikeLo),
            quantize(strikeHi),
            quantize(tau)} {}

std::int64_t PricingParams::quantize(const double param) noexcept {
  static constexpr double scale{1e6};  // 1e-6 precision
  return static_cast<std::int64_t>(param * scale);
}

bool PricingParams::operator==(const PricingParams& other) const noexcept {
  return data_ == other.data_;
}

void PricingParamsHash::hashCombine(std::size_t& seed,
                                    const std::size_t v) noexcept {
  // 64-bit fractional part of the golden ratio
  constexpr unsigned long long goldenRatio{0x9e3779b97f4a7c15ULL};
  seed ^= v + goldenRatio + (seed << 6) + (seed >> 2);
}

std::size_t PricingParamsHash::operator()(
    const PricingParams& params) const noexcept {
  static constexpr std::hash<std::int64_t> hasher{};
  std::size_t seed{0};

  for (const auto param : params.data_) {
    hashCombine(seed, hasher(param));
  }

  return seed;
}
