// SPDX-License-Identifier: MIT
/// @file bench_lru.cpp
/// @brief Micro-benchmarks for lru::LruCache.

#include <benchmark/benchmark.h>

#include <cstddef>
#include <random>
#include <string>

#include "lru/lru_cache.hpp"

namespace {

/// Zipf-ish access pattern: mostly hits, a few cold misses.
void run_workload(benchmark::State& state, std::size_t capacity, double hit_ratio) {
    lru::LruCache<int, int> cache(capacity);
    std::mt19937 rng(42);
    std::uniform_real_distribution<double> coin(0.0, 1.0);
    std::uniform_int_distribution<int> hot(0, static_cast<int>(capacity) - 1);
    std::uniform_int_distribution<int> cold(0, static_cast<int>(capacity) * 4);

    for (int i = 0; i < static_cast<int>(capacity); ++i) {
        cache.put(i, i);
    }

    for (auto _ : state) {
        const int key = coin(rng) < hit_ratio ? hot(rng) : cold(rng);
        const auto value = cache.get(key);
        benchmark::DoNotOptimize(value);
        cache.put(key, key);
    }

    state.counters["hits"] = static_cast<double>(cache.stats().hits);
    state.counters["misses"] = static_cast<double>(cache.stats().misses);
    state.counters["evictions"] = static_cast<double>(cache.stats().evictions);
}

}  // namespace

BENCHMARK_CAPTURE(run_workload, small_hot, 1024, 0.95)->Unit(benchmark::kMicrosecond);
BENCHMARK_CAPTURE(run_workload, medium_mixed, 8192, 0.80)->Unit(benchmark::kMicrosecond);
BENCHMARK_CAPTURE(run_workload, large_cold, 65536, 0.50)->Unit(benchmark::kMicrosecond);

BENCHMARK_MAIN();
