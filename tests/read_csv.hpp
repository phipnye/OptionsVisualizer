#pragma once

#include <Eigen/Dense>
#include <charconv>
#include <cstddef>
#include <filesystem>
#include <fstream>
#include <stdexcept>
#include <string>
#include <string_view>

// Assume we know the number of rows and columns so we don't have to use a
// vector and then steal the buffers
[[nodiscard]] inline Eigen::ArrayXXd readCSV(const std::string_view file,
                                             const Eigen::Index nrow,
                                             const Eigen::Index ncol) {
  std::ifstream in{std::filesystem::path{file}};
  std::string line{};
  Eigen::ArrayXXd res{Eigen::ArrayXXd::Zero(nrow, ncol)};

  if (!in.is_open()) {
    throw std::runtime_error{"Unable to open file: " + std::string{file}};
  }

  for (Eigen::Index row{0}; row < nrow && std::getline(in, line); ++row) {
    if (!line.empty() && line.back() == '\r') {
      line.pop_back();
    }

    line += ',';  // add trailing comma to extract the last column
    const std::string_view view{line};
    const std::size_t nChar{view.size()};
    std::size_t lPtr{0};
    Eigen::Index col{0};

    for (std::size_t rPtr{0}; col < ncol && rPtr < nChar; ++rPtr) {
      if (view[rPtr] == ',') {
        const std::string_view sub{view.substr(lPtr, rPtr - lPtr)};
        const auto [ptr, ec]{std::from_chars(
            sub.data(), sub.data() + sub.size(), res(row, col))};

        if (ec != std::errc{}) {
          throw std::runtime_error{
              "Conversion error at [" + std::to_string(row) + ", " +
              std::to_string(col) + "in file: " + std::string{file}};
        }

        ++col;
        lPtr = rPtr + 1;
      }
    }
  }

  return res;
}
