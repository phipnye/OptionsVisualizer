#include "OptionsVisualizer/lru/LRUCache.hpp"
#include <cstddef>

LRUCache::LRUCache(std::size_t capacity) : capacity_{capacity} {
    cache_.reserve(capacity_);
}
