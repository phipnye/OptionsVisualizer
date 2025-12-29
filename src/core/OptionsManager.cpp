#include "OptionsVisualizer/core/OptionsManager.hpp"

#include <Eigen/Dense>
#include <algorithm>
#include <array>
#include <cstddef>
#include <list>
#include <unordered_map>
#include <utility>

#include "BS_thread_pool.hpp"
#include "OptionsVisualizer/core/globals.hpp"
#include "OptionsVisualizer/lru/LRUCache.hpp"
#include "OptionsVisualizer/lru/Params.hpp"
#include "OptionsVisualizer/pricing/PricingSurface.hpp"

// Constructs a thread pool with as many threads as are available in the
// hardware
OptionsManager::OptionsManager(const std::size_t capacity)
    : LRUCache{std::max(capacity, 1UL)}, pool_{} {}

// Constructs a thread pool with specified number of threads
OptionsManager::OptionsManager(const std::size_t capacity,
                               const std::size_t nThreads)
    : LRUCache{std::max(capacity, 1UL)}, pool_{std::max(nThreads, 1UL)} {}

// Retrieve cached greek values or compute new ones and cache the results
const std::array<Eigen::MatrixXd, globals::nGrids>& OptionsManager::get(
    const Eigen::DenseIndex nSigma, const Eigen::DenseIndex nStrike,
    const double spot, const double r, const double q, const double sigmaLo,
    const double sigmaHi, const double strikeLo, const double strikeHi,
    const double tau) {
  const Params params{nSigma,  nStrike, spot,     r,        q,
                      sigmaLo, sigmaHi, strikeLo, strikeHi, tau};

  // Already computed the given value
  if (const auto search{cache_.find(params)}; search != cache_.cend()) {
    const auto& [val, oldIt]{search->second};
    keys_.splice(keys_.end(), keys_, oldIt);
    return val;
  }

  // Compute value if not available
  const PricingSurface surface{nSigma,  nStrike,  spot,     r,   q,    sigmaLo,
                               sigmaHi, strikeLo, strikeHi, tau, pool_};
  std::array<Eigen::MatrixXd, globals::nGrids> grids{surface.calculateGrids()};

  // Move the result into the cache
  set(params, std::move(grids));
  return cache_[params].first;
}

void OptionsManager::set(const Params& params,
                         std::array<Eigen::MatrixXd, globals::nGrids>&& grids) {
  // No need to consider overwriting pre-existing keys since results will be
  // identical and the result would've already been returned

  // Remove least recently used if we're at max capacity
  if (cache_.size() == capacity_) {
    cache_.erase(keys_.front());
    keys_.pop_front();
  }

  // Add new entry
  keys_.push_back(params);
  cache_.emplace(params,
                 std::make_pair(std::move(grids), std::prev(keys_.end())));
}
