#pragma once

#include <Eigen/Dense>
// ReSharper disable once CppUnusedIncludeDirective
#include <cstdint>

struct PricingParams {
  std::int64_t nSigma_;
  std::int64_t nStrike_;
  std::int64_t spot_;
  std::int64_t r_;
  std::int64_t q_;
  std::int64_t sigmaLo_;
  std::int64_t sigmaHi_;
  std::int64_t strikeLo_;
  std::int64_t strikeHi_;
  std::int64_t tau_;

  PricingParams(Eigen::Index nSigma, Eigen::Index nStrike, double spot,
                double r, double q, double sigmaLo, double sigmaHi,
                double strikeLo, double strikeHi, double tau);

  bool operator==(const PricingParams& other) const noexcept;

 private:
  // Scale doubles to 64-bit integers
  static constexpr double scale{1e6};  // 1e-6 precision
  [[nodiscard]] static std::int64_t quantize(double param);
};
