#pragma once

#include <Eigen/Dense>
#include <cstdint>

struct Params {
    std::int64_t nSigma;
    std::int64_t nStrike;
    std::int64_t spot;
    std::int64_t r;
    std::int64_t q;
    std::int64_t sigmaLo;
    std::int64_t sigmaHi;
    std::int64_t strikeLo;
    std::int64_t strikeHi;
    std::int64_t tau;

    Params(Eigen::DenseIndex nSigma_, Eigen::DenseIndex nStrike_, double spot_, double r_, double q_, double sigmaLo_,
           double sigmaHi_, double strikeLo_, double strikeHi_, double tau_);

private:
    static constexpr double scale{1e6}; // 1e-6 precision
    static std::int64_t quantize(double param);
};
