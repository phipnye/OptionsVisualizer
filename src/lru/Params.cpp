#include "OptionsVisualizer/lru/Params.hpp"
#include <Eigen/Dense>

Params::Params(const Eigen::DenseIndex nSigma_, const Eigen::DenseIndex nStrike_, const double spot_, const double r_,
               const double q_,
               const double sigmaLo_, const double sigmaHi_, const double strikeLo_, const double strikeHi_,
               const double tau_)
    : nSigma{static_cast<std::int64_t>(nSigma_)}, nStrike{static_cast<std::int64_t>(nStrike_)}, spot{quantize(spot_)},
      r{quantize(r_)}, q{quantize(q_)}, sigmaLo{quantize(sigmaLo_)}, sigmaHi{quantize(sigmaHi_)},
      strikeLo{quantize(strikeLo_)}, strikeHi{quantize(strikeHi_)}, tau{quantize(tau_)} {
}

std::int64_t Params::quantize(const double param) {
    return static_cast<std::int64_t>(param * scale);
}

bool Params::operator==(const Params &other) const noexcept {
    return nSigma == other.nSigma && nStrike == other.nStrike && spot == other.spot && r == other.r && q == other.q &&
           sigmaLo == other.sigmaLo && sigmaHi == other.sigmaHi && strikeLo == other.strikeLo &&
           strikeHi == other.strikeHi && tau == other.tau;
}
