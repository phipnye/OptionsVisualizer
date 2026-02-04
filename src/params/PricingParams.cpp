#include "OptionsVisualizer/params/PricingParams.hpp"

#include <Eigen/Dense>

PricingParams::PricingParams(const Eigen::Index nSigma,
                             const Eigen::Index nStrike, const double spot,
                             const double r, const double q,
                             const double sigmaLo, const double sigmaHi,
                             const double strikeLo, const double strikeHi,
                             const double tau)
    : nSigma_{static_cast<std::int64_t>(nSigma)},
      nStrike_{static_cast<std::int64_t>(nStrike)},
      spot_{quantize(spot)},
      r_{quantize(r)},
      q_{quantize(q)},
      sigmaLo_{quantize(sigmaLo)},
      sigmaHi_{quantize(sigmaHi)},
      strikeLo_{quantize(strikeLo)},
      strikeHi_{quantize(strikeHi)},
      tau_{quantize(tau)} {}

std::int64_t PricingParams::quantize(const double param) {
  return static_cast<std::int64_t>(param * scale);
}

bool PricingParams::operator==(const PricingParams& other) const noexcept {
  return nSigma_ == other.nSigma_ && nStrike_ == other.nStrike_ &&
         spot_ == other.spot_ && r_ == other.r_ && q_ == other.q_ &&
         sigmaLo_ == other.sigmaLo_ && sigmaHi_ == other.sigmaHi_ &&
         strikeLo_ == other.strikeLo_ && strikeHi_ == other.strikeHi_ &&
         tau_ == other.tau_;
}
