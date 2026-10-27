// SPDX-License-Identifier: MIT
/// @file test_lru.cpp
/// @brief Unit tests for lru::LruCache.

#include <gtest/gtest.h>

#include <atomic>
#include <string>
#include <thread>
#include <vector>

#include "lru/lru_cache.hpp"

TEST(LruCache, PutThenGet) {
    lru::LruCache<int, std::string> cache(2);
    cache.put(1, "one");
    EXPECT_EQ(cache.get(1), std::optional<std::string>("one"));
    EXPECT_FALSE(cache.get(2).has_value());
}

TEST(LruCache, EvictsLeastRecentlyUsed) {
    lru::LruCache<int, int> cache(2);
    cache.put(1, 10);
    cache.put(2, 20);
    cache.get(1);             // 1 becomes most recent, 2 is now LRU
    cache.put(3, 30);         // evicts 2
    EXPECT_TRUE(cache.get(1).has_value());
    EXPECT_FALSE(cache.get(2).has_value());
    EXPECT_TRUE(cache.get(3).has_value());
    EXPECT_EQ(cache.stats().evictions, 1u);
}

TEST(LruCache, UpdatingExistingKeyDoesNotGrow) {
    lru::LruCache<int, int> cache(2);
    cache.put(1, 10);
    cache.put(1, 11);
    EXPECT_EQ(cache.size(), 1u);
    EXPECT_EQ(cache.get(1), std::optional<int>(11));
    EXPECT_EQ(cache.stats().evictions, 0u);
}

TEST(LruCache, StatsCountHitsAndMisses) {
    lru::LruCache<int, int> cache(2);
    cache.put(1, 1);
    cache.get(1);
    cache.get(1);
    cache.get(42);
    const auto stats = cache.stats();
    EXPECT_EQ(stats.hits, 2u);
    EXPECT_EQ(stats.misses, 1u);
}

TEST(LruCache, ZeroCapacityKeepsNothing) {
    lru::LruCache<int, int> cache(0);
    cache.put(1, 1);
    EXPECT_EQ(cache.size(), 0u);
    EXPECT_FALSE(cache.get(1).has_value());
}

TEST(LruCache, EraseRemovesEntry) {
    lru::LruCache<int, int> cache(2);
    cache.put(1, 1);
    EXPECT_TRUE(cache.erase(1));
    EXPECT_FALSE(cache.erase(1));
    EXPECT_FALSE(cache.contains(1));
}

TEST(LruCache, ClearDropsEntriesButKeepsStats) {
    lru::LruCache<int, int> cache(2);
    cache.put(1, 1);
    cache.get(1);
    cache.clear();
    EXPECT_EQ(cache.size(), 0u);
    EXPECT_EQ(cache.stats().hits, 1u);
}

TEST(LruCache, ConcurrentAccessKeepsInvariants) {
    lru::LruCache<int, int> cache(64);
    std::atomic<int> failures{0};
    constexpr int kThreads = 4;
    constexpr int kOps = 2000;

    std::vector<std::thread> workers;
    workers.reserve(kThreads);
    for (int t = 0; t < kThreads; ++t) {
        workers.emplace_back([&cache, &failures, t] {
            for (int i = 0; i < kOps; ++i) {
                const int key = (i * 7 + t) % 128;
                cache.put(key, key * 2);
                const auto value = cache.get(key);
                if (!value.has_value() || *value != key * 2) {
                    ++failures;
                }
            }
        });
    }
    for (auto& worker : workers) {
        worker.join();
    }

    EXPECT_EQ(failures.load(), 0);
    EXPECT_LE(cache.size(), cache.capacity());
}
