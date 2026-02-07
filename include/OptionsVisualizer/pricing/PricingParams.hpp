#pragma once

#include <Eigen/Dense>
#include <array>
#include <cstddef>
#include <cstdint>

// Hashable wrapper around an std::array
class PricingParams {
  friend class PricingParamsHash;
  static constexpr std::size_t nParams{10};
  std::array<std::int64_t, nParams> data_;

 public:
  PricingParams(Eigen::Index nSigma, Eigen::Index nStrike, double spot,
                double r, double q, double sigmaLo, double sigmaHi,
                double strikeLo, double strikeHi, double tau);

  bool operator==(const PricingParams& other) const noexcept;

 private:
  // Scale doubles to 64-bit integers
  [[nodiscard]] static std::int64_t quantize(double param) noexcept;
};

// Hashing "struct" for PricingParams (not marking as struct because some
// implementations may complain about friend structs)
class PricingParamsHash {
 public:
  [[nodiscard]] std::size_t operator()(
      const PricingParams& params) const noexcept;

  // 64-bit version of boost::hash_combine
  // (https://www.boost.org/doc/libs/1_36_0/doc/html/hash/reference.html#boost.hash_combine)
  static void hashCombine(std::size_t& seed, std::size_t v) noexcept;
};
