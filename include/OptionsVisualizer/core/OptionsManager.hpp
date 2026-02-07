#pragma once

#include <BS_thread_pool.hpp>
#include <Eigen/Dense>
#include <array>
#include <cstddef>

#include "OptionsVisualizer/core/globals.hpp"
#include "OptionsVisualizer/lru/LRUCache.hpp"
#include "OptionsVisualizer/pricing/PricingParams.hpp"

// Class exported to python for generating greek results across a grid of sigma
// x strike (it is in charge of caching results using least recently used
// methodology and managing the thread pool). Our module actually exports the
// raw data buffers managed by the object rather than copying the results which
// would lead to dangling references if cached results were deleted prior to
// python's use; however, we only view one set of results at a time so this
// isn't a concern for this project
class OptionsManager {
  //--- Data members

  // LRU cache
  using GridArray = std::array<Eigen::ArrayXXd, globals::nGrids>;
  LRUCache<PricingParams, GridArray, PricingParamsHash> lru_;

  // Thread pool for trinomial pricing
  BS::thread_pool<> pool_;

 public:
  // Constructs a thread pool with total number of threads available on hardware
  explicit OptionsManager(std::size_t capacity);

  // Constructs a thread pool with a specified number of threads
  explicit OptionsManager(std::size_t capacity, std::size_t nThreads);

  // Retrieve cached greek values or compute new ones and cache the results
  [[nodiscard]] const GridArray& get(Eigen::Index nSigma, Eigen::Index nStrike,
                                     double spot, double r, double q,
                                     double sigmaLo, double sigmaHi,
                                     double strikeLo, double strikeHi,
                                     double tau);
};
