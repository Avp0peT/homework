// SPDX-License-Identifier: MIT
/// @file lru_cache.hpp
/// @brief A small thread-safe LRU cache with hit/miss statistics.
#pragma once

#include <cstddef>
#include <list>
#include <mutex>
#include <optional>
#include <unordered_map>
#include <utility>

namespace lru {

/// Cache counters, updated on every lookup and eviction.
struct Stats {
    std::size_t hits{0};       ///< lookups that found a live entry
    std::size_t misses{0};     ///< lookups that did not
    std::size_t evictions{0};  ///< entries dropped because capacity was exceeded
};

/// Fixed-capacity least-recently-used cache.
///
/// @tparam Key   key type, must be hashable by @p Hash and copyable
/// @tparam Value value type, copied on insert and returned by value on lookup
/// @tparam Hash  hash functor for @p Key
///
/// All public member functions are thread-safe: a single mutex guards the
/// list and the index. This trades throughput for simplicity on purpose; see
/// the benchmark for the numbers.
template <class Key, class Value, class Hash = std::hash<Key>>
class LruCache {
public:
    /// @param capacity maximum number of live entries; 0 makes the cache a black hole
    explicit LruCache(std::size_t capacity) : capacity_(capacity) {}

    LruCache(const LruCache&) = delete;
    LruCache& operator=(const LruCache&) = delete;
    LruCache(LruCache&&) = delete;
    LruCache& operator=(LruCache&&) = delete;

    /// Look up @p key and mark it most-recently-used on a hit.
    /// @return the stored value, or std::nullopt when the key is absent
    std::optional<Value> get(const Key& key) {
        std::lock_guard<std::mutex> guard(mutex_);
        auto it = index_.find(key);
        if (it == index_.end()) {
            ++stats_.misses;
            return std::nullopt;
        }
        ++stats_.hits;
        items_.splice(items_.begin(), items_, it->second);
        return it->second->second;
    }

    /// Insert or update @p key. Evicts the least-recently-used entry when full.
    void put(const Key& key, Value value) {
        if (capacity_ == 0) {
            return;
        }
        std::lock_guard<std::mutex> guard(mutex_);
        auto it = index_.find(key);
        if (it != index_.end()) {
            it->second->second = std::move(value);
            items_.splice(items_.begin(), items_, it->second);
            return;
        }
        items_.emplace_front(key, std::move(value));
        index_.emplace(key, items_.begin());
        if (items_.size() > capacity_) {
            auto last = std::prev(items_.end());
            index_.erase(last->first);
            items_.pop_back();
            ++stats_.evictions;
        }
    }

    /// Remove @p key if present.
    /// @return true when something was erased
    bool erase(const Key& key) {
        std::lock_guard<std::mutex> guard(mutex_);
        auto it = index_.find(key);
        if (it == index_.end()) {
            return false;
        }
        items_.erase(it->second);
        index_.erase(it);
        return true;
    }

    /// @return true when @p key is present, without changing recency
    bool contains(const Key& key) const {
        std::lock_guard<std::mutex> guard(mutex_);
        return index_.find(key) != index_.end();
    }

    /// Drop every entry; counters are preserved.
    void clear() {
        std::lock_guard<std::mutex> guard(mutex_);
        items_.clear();
        index_.clear();
    }

    /// @return number of live entries
    std::size_t size() const {
        std::lock_guard<std::mutex> guard(mutex_);
        return items_.size();
    }

    /// @return configured capacity
    std::size_t capacity() const { return capacity_; }

    /// @return a snapshot of the counters
    Stats stats() const {
        std::lock_guard<std::mutex> guard(mutex_);
        return stats_;
    }

private:
    using Entry = std::pair<Key, Value>;
    using EntryIt = typename std::list<Entry>::iterator;

    mutable std::mutex mutex_;
    const std::size_t capacity_;
    std::list<Entry> items_;  ///< front = most recently used
    std::unordered_map<Key, EntryIt, Hash> index_;
    Stats stats_{};
};

}  // namespace lru
