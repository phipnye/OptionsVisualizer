#pragma once

#include <cstddef>
#include <list>
#include <stdexcept>
#include <unordered_map>

template <typename Key, typename Value, typename KeyHash>
class LRUCache {
  struct Entry {
    std::list<Key>::iterator iter;
    Value value;
  };

  // --- Data members
  std::unordered_map<Key, Entry, KeyHash> cache_{};
  std::list<Key> keys_{};
  std::size_t capacity_;

 public:
  // Ctor
  explicit LRUCache(const std::size_t capacity) : capacity_{capacity} {
    cache_.reserve(capacity_);
  }

  // See if a key-value pairing exists (doesn't modify LRU logic)
  [[nodiscard]] bool contains(const Key& key) const {
    return cache_.contains(key);
  }

  // Retrieve values
  [[nodiscard]] const Value& get(const Key& key) {
    if (const auto search{cache_.find(key)}; search != cache_.cend()) {
      // LRU logic, mark retrieved value as last used
      const auto& [oldIter, val]{search->second};
      keys_.splice(keys_.end(), keys_, oldIter);
      return val;
    }

    // For our use case, the logic to get here should be impossible since we
    // check whether the container contains the value and calculate then store
    // the results prior to calling get if not already stored
    throw std::out_of_range{"Cannot find specified key in cache"};
  }

  // Store values
  void set(const Key& key, Value&& val) {
    // We do not need to check if the value already exists in our use case
    // since a cached result would already be returned before any call to set

    // Delete old values if no more space
    if (cache_.size() == capacity_) {
      cache_.erase(keys_.front());
      keys_.pop_front();
    }

    // Mark set value as last used
    keys_.push_back(key);
    cache_.emplace(
        key, Entry{.iter = std::prev(keys_.end()), .value = std::move(val)});
  }
};
