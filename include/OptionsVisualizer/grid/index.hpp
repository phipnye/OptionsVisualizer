#pragma once

#include "OptionsVisualizer/greeks/GreeksResult.hpp"
#include "OptionsVisualizer/grid/Dims.hpp"
#include <unsupported/Eigen/CXX11/Tensor>

namespace grid::index {

enum class GreekType : Eigen::DenseIndex { Price, Delta, Gamma, Vega, Theta };
enum class OptionType : Eigen::DenseIndex { AmerCall, AmerPut, EuroCall, EuroPut };

constexpr Eigen::DenseIndex idx(OptionType t) noexcept {
    return static_cast<Eigen::DenseIndex>(t);
}

constexpr Eigen::DenseIndex idx(GreekType t) noexcept {
    return static_cast<Eigen::DenseIndex>(t);
}

void writeOptionGreeks(Eigen::VectorXd& output, index::OptionType option, const greeks::GreeksResult& g, Dims dims);

} // namespace grid::index
