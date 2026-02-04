#pragma once

// ReSharper disable once CppUnusedIncludeDirective
#include <Eigen/Dense>
#include <array>
// ReSharper disable once CppUnusedIncludeDirective
#include <cstddef>

#include "BS_thread_pool.hpp"
#include "OptionsVisualizer/core/globals.hpp"
#include "OptionsVisualizer/lru/LRUCache.hpp"
#include "OptionsVisualizer/params/PricingParams.hpp"
#include "OptionsVisualizer/params/PricingParamsHash.hpp"

class OptionsManager {
  using GridArray = std::array<Eigen::ArrayXXd, globals::nGrids>;

  //--- Data members

  // LRU cache
  LRUCache<PricingParams, GridArray, PricingParamsHash> lru_;

  // Thread pool for trinomial pricing
  BS::thread_pool<> pool_;

 public:
  // Constructs a thread pool with number of threads available on hardware
  explicit OptionsManager(std::size_t capacity);

  // Constructs a thread pool with specified number of threads
  explicit OptionsManager(std::size_t capacity, std::size_t nThreads);

  // Retrieve cached greek values or compute new ones and cache the results
  [[nodiscard]] const GridArray& get(Eigen::Index nSigma, Eigen::Index nStrike,
                                     double spot, double r, double q,
                                     double sigmaLo, double sigmaHi,
                                     double strikeLo, double strikeHi,
                                     double tau);
};
