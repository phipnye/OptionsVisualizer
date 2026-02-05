#include <Eigen/Dense>
#include <filesystem>
#include <iostream>
#include <regex>

#include "OptionsVisualizer/core/OptionsManager.hpp"
#include "align_files.hpp"
#include "gtest/gtest.h"
#include "read_csv.hpp"

TEST(PricingTests, ValidateResults) {
  // Important this matches with python test generation
  constexpr Eigen::DenseIndex nrow{10};
  constexpr Eigen::DenseIndex ncol{10};

  // Read in tests - Using ASSERT ensures we stop if data is missing
  const std::filesystem::path dataPath{TEST_DATA_PATH};
  const Eigen::ArrayXXd s{readCSV((dataPath / "s.csv").c_str(), nrow, ncol)};
  const Eigen::ArrayXXd k{readCSV((dataPath / "k.csv").c_str(), nrow, ncol)};
  const Eigen::ArrayXXd r{readCSV((dataPath / "r.csv").c_str(), nrow, ncol)};
  const Eigen::ArrayXXd q{readCSV((dataPath / "q.csv").c_str(), nrow, ncol)};
  const Eigen::ArrayXXd t{readCSV((dataPath / "t.csv").c_str(), nrow, ncol)};
  const Eigen::ArrayXXd sigma{
      readCSV((dataPath / "sigma.csv").c_str(), nrow, ncol)};

  // Instatiate a manager object to retrieve results
  constexpr std::size_t lruCapacity{nrow * ncol};  // cache all of the results
  constexpr std::size_t nThreads{1};
  OptionsManager manager{lruCapacity, nThreads};

  // Iterate through python results files
  const std::regex pyResFilePattern{"^(amer|euro)_(call|put)_.+$"};

  for (const auto& entry : std::filesystem::directory_iterator(dataPath)) {
    if (!std::filesystem::is_regular_file(entry)) {
      continue;
    }

    if (const std::string fileStem{entry.path().stem().string()};
        std::regex_match(fileStem, pyResFilePattern)) {
      const Eigen::ArrayXXd pyResults{
          readCSV(entry.path().c_str(), nrow, ncol)};
      const std::size_t idx{greekIndexFromFilename(fileStem)};

      for (Eigen::Index col{0}; col < ncol; ++col) {
        for (Eigen::Index row{0}; row < nrow; ++row) {
          // Extract results from the manager
          constexpr Eigen::Index surfaceSize{1};
          const auto& grids{manager.get(surfaceSize, surfaceSize, s(row, col),
                                        r(row, col), q(row, col),
                                        sigma(row, col), sigma(row, col),
                                        k(row, col), k(row, col), t(row, col))};

          // Compare python and c++ results
          const double cppVal{grids[idx](0, 0)};
          const double pyVal{pyResults(row, col)};
          EXPECT_NEAR(cppVal, pyVal, 1e-6)
              << "Failure in file: " << fileStem << " at: [" << row << ", "
              << col << "]";
        }
      }
    }
  }
}
