#pragma once

#include <Eigen/Dense>

// Enums for determing type of option
enum class GreekType : Eigen::DenseIndex { Price = 0, Delta = 1, Gamma = 2, Vega = 3, Theta = 4, Rho = 5, COUNT = 6 };
enum class OptionType : Eigen::DenseIndex { AmerCall = 0, AmerPut = 1, EuroCall = 2, EuroPut = 3, COUNT = 4 };

static constexpr Eigen::DenseIndex idx(OptionType t) noexcept {
    return static_cast<Eigen::DenseIndex>(t);
}

static constexpr Eigen::DenseIndex idx(GreekType t) noexcept {
    return static_cast<Eigen::DenseIndex>(t);
}
