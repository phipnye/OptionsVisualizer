#include <Eigen/Dense>
#include <filesystem>
#include <iostream>
#include <regex>

#include "OptionsVisualizer/core/OptionsManager.hpp"
#include "align_files.hpp"
#include "gtest/gtest.h"
#include "read_data.hpp"

TEST(PricingTests, ValidateResults) {
  const std::filesystem::path dataPath{TEST_DATA_PATH};

  // Determine number of testing observations
  const Eigen::DenseIndex nrow{
      getTestSize((dataPath / "input_test_size.txt").c_str())};
  constexpr Eigen::DenseIndex ncol{1};

  // Read in tests parameters
  const Eigen::ArrayXXd s{readCSV((dataPath / "s.csv").c_str(), nrow, ncol)};
  const Eigen::ArrayXXd k{readCSV((dataPath / "k.csv").c_str(), nrow, ncol)};
  const Eigen::ArrayXXd r{readCSV((dataPath / "r.csv").c_str(), nrow, ncol)};
  const Eigen::ArrayXXd q{readCSV((dataPath / "q.csv").c_str(), nrow, ncol)};
  const Eigen::ArrayXXd t{readCSV((dataPath / "t.csv").c_str(), nrow, ncol)};
  const Eigen::ArrayXXd sigma{
      readCSV((dataPath / "sigma.csv").c_str(), nrow, ncol)};

  // Instatiate a manager object to retrieve results (cache all of the results)
  constexpr std::size_t nThreads{1};
  const std::size_t lruCapacity{static_cast<std::size_t>(nrow * ncol)};
  OptionsManager manager{lruCapacity, nThreads};

  // Iterate through python results files
  const std::regex pyResFilePattern{"^(amer|euro)_(call|put)_[a-z]+$"};

  for (const auto& entry : std::filesystem::directory_iterator(dataPath)) {
    const std::string fileStem{entry.path().stem().string()};

    if (!std::regex_match(fileStem, pyResFilePattern)) {
      continue;
    }

    const Eigen::ArrayXXd pyResults{readCSV(entry.path().c_str(), nrow, ncol)};
    const std::size_t idx{greekIndexFromFilename(fileStem)};

    for (Eigen::Index col{0}; col < ncol; ++col) {
      for (Eigen::Index row{0}; row < nrow; ++row) {
        // Extract results from the manager
        constexpr Eigen::Index surfaceSize{1};
        const auto& grids{manager.get(surfaceSize, surfaceSize, s(row, col),
                                      r(row, col), q(row, col), sigma(row, col),
                                      sigma(row, col), k(row, col), k(row, col),
                                      t(row, col))};

        // Compare python and c++ results
        const double cppVal{grids[idx](0, 0)};
        const double pyVal{pyResults(row, col)};
        EXPECT_NEAR(cppVal, pyVal, 1e-4)
            << "Failure in file: " << fileStem << " at: [" << row << ", " << col
            << "]";
      }
    }
  }
}
