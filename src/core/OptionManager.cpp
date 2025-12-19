#include "OptionsVisualizer/core/OptionManager.hpp"
#include "OptionsVisualizer/core/globals.hpp"
#include "OptionsVisualizer/lru/LRUCache.hpp"
#include "OptionsVisualizer/lru/Params.hpp"
#include "OptionsVisualizer/pricing/PricingSurface.hpp"
#include <Eigen/Dense>
#include <algorithm>
#include <array>
#include <cstddef>
#include <list>
#include <unordered_map>
#include <utility>

OptionManager::OptionManager(std::size_t capacity) : LRUCache{std::max(capacity, 1UL)} {}

const std::array<Eigen::MatrixXd, globals::nGrids>& OptionManager::get(Eigen::DenseIndex nSigma,
                                                                       Eigen::DenseIndex nStrike, double spot, double r,
                                                                       double q, double sigmaLo, double sigmaHi,
                                                                       double strikeLo, double strikeHi, double tau) {
    const Params params{nSigma, nStrike, spot, r, q, sigmaLo, sigmaHi, strikeLo, strikeHi, tau};

    // Already computed the given value
    if (auto search{cache_.find(params)}; search != cache_.cend()) {
        const auto& [val, oldIt]{search->second};
        keys_.splice(keys_.end(), keys_, oldIt);
        return val;
    }

    // Compute value if not available
    const PricingSurface surface{nSigma, nStrike, spot, r, q, sigmaLo, sigmaHi, strikeLo, strikeHi, tau};
    std::array<Eigen::MatrixXd, globals::nGrids> computedValue{surface.calculateGrids()};

    // Move the result into the cache
    set(params, std::move(computedValue));
    return cache_[params].first;
}

void OptionManager::set(const Params& params, std::array<Eigen::MatrixXd, globals::nGrids>&& value) {
    // No need to consider overwriting pre-existing keys since results will be identical and the result would've already
    // been returned

    // Remove least recently used if we're at max capacity
    if (cache_.size() == capacity_) {
        cache_.erase(keys_.front());
        keys_.pop_front();
    }

    // Add new entry
    keys_.push_back(params);
    cache_.emplace(params, std::make_pair(std::move(value), std::prev(keys_.end())));
}
