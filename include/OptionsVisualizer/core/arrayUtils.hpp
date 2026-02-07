#pragma once

#include <Eigen/Dense>
#include <array>
#include <cstddef>
#include <utility>

namespace Utils {

namespace Internal {

// Internal helper for creating a std::array of Eigen array's with pre-allocated
// buffer sizes
template <std::size_t... IDXs>
std::array<Eigen::ArrayXXd, sizeof...(IDXs)> preallocArrays(
    const Eigen::Index nrow, const Eigen::Index ncol,
    std::index_sequence<IDXs...>) {
  return {(static_cast<void>(IDXs), Eigen::ArrayXXd{nrow, ncol})...};
}

}  // namespace Internal

// Construct a std::array of Eigen array's with pre-allocated buffer sizes
template <std::size_t N>
std::array<Eigen::ArrayXXd, N> preallocArrays(const Eigen::Index nrow,
                                              const Eigen::Index ncol) {
  return Internal::preallocArrays(nrow, ncol, std::make_index_sequence<N>{});
}

}  // namespace Utils
