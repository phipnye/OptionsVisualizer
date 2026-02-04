#pragma once

#include <cstddef>
#include <list>
#include <stdexcept>
#include <unordered_map>
#include <utility>

template <typename Key, typename StoredType, typename KeyHash>
class LRUCache {
  // --- Data members
  std::unordered_map<
      Key, std::pair<StoredType, typename std::list<Key>::iterator>, KeyHash>
      cache_{};
  std::list<Key> keys_{};
  std::size_t capacity_;

 public:
  // Ctor
  explicit LRUCache(const std::size_t capacity) : capacity_{capacity} {
    cache_.reserve(capacity_);
  }

  // See if a key-value pairing exists
  [[nodiscard]] bool contains(const Key& key) const {
    return cache_.contains(key);
  }

  // Retrieve values
  [[nodiscard]] const StoredType& get(const Key& key) {
    if (const auto search{cache_.find(key)}; search != cache_.cend()) {
      const auto& [val, oldIt]{search->second};
      keys_.splice(keys_.end(), keys_, oldIt);
      return val;
    }

    throw std::out_of_range{"Cannot find specified key in cache"};
  }

  // Store values
  void set(const Key& key, StoredType&& val) {
    // Delete old values if no more space
    if (cache_.size() == capacity_) {
      cache_.erase(keys_.front());
      keys_.pop_front();
    }

    keys_.push_back(key);
    cache_.emplace(key, std::make_pair(std::move(val), std::prev(keys_.end())));
  }
};
