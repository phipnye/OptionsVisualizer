#include "OptionsVisualizer/core/OptionsManager.hpp"

#include <BS_thread_pool.hpp>
#include <Eigen/Dense>
#include <algorithm>
#include <array>
#include <cstddef>
#include <list>

#include "OptionsVisualizer/pricing/PricingParams.hpp"
#include "OptionsVisualizer/lru/LRUCache.hpp"
#include "OptionsVisualizer/pricing/PricingSurface.hpp"

// Constructs a thread pool with number of threads available on hardware
OptionsManager::OptionsManager(const std::size_t capacity)
    : lru_{std::max(capacity, std::size_t{1})}, pool_{} {}

// Constructs a thread pool with specified number of threads
OptionsManager::OptionsManager(const std::size_t capacity,
                               const std::size_t nThreads)
    : lru_{std::max(capacity, std::size_t{1})},
      pool_{std::max(nThreads, std::size_t{1})} {}

// Retrieve cached greek values or compute new ones and cache the results
const OptionsManager::GridArray& OptionsManager::get(
    const Eigen::Index nSigma, const Eigen::Index nStrike, const double spot,
    const double r, const double q, const double sigmaLo, const double sigmaHi,
    const double strikeLo, const double strikeHi, const double tau) {
  // Can't pass these directly to PricingSurface ctor since doubles get
  // quantized to integers
  const PricingParams params{nSigma,  nStrike, spot,     r,        q,
                             sigmaLo, sigmaHi, strikeLo, strikeHi, tau};

  // Compute new value if not stored
  if (!lru_.contains(params)) {
    const PricingSurface surface{nSigma,   nStrike, spot,    r,
                                 q,        sigmaLo, sigmaHi, strikeLo,
                                 strikeHi, tau,     pool_};
    lru_.set(params, surface.calculateGrids());
  }

  return lru_.get(params);
}
