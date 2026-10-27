# lru-cache

A small, header-only, thread-safe LRU cache written in C++20, with unit tests,
micro-benchmarks, generated API documentation and a GitHub Actions pipeline that
builds, tests, benchmarks and publishes the docs on every push.

**Documentation:** https://avp0pet.github.io/homework/

## What it does

`lru::LruCache<Key, Value>` keeps at most *N* entries. Reading an entry marks it
as most recently used; inserting into a full cache drops the least recently used
entry. All operations are guarded by a single mutex, which keeps the
implementation small and predictable. Hit, miss and eviction counters are
exposed through `stats()`.

## Layout

```
lru-cache/
├── include/lru/lru_cache.hpp   # the library (header-only)
├── tests/test_lru.cpp          # GoogleTest unit tests
├── benchmarks/bench_lru.cpp    # Google Benchmark micro-benchmarks
├── docs/Doxyfile.in            # Doxygen template, filled in by CMake
├── .github/workflows/ci.yml    # build + test + benchmark + docs
├── CMakeLists.txt
├── AGENTS.md                   # instructions for AI coding agents
└── CLAUDE.md                   # thin wrapper that points at AGENTS.md
```

## Build

Requirements: CMake ≥ 3.20, a C++20 compiler. GoogleTest and Google Benchmark
are fetched automatically by CMake.

```bash
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build --parallel
```

## Test

```bash
ctest --test-dir build --output-on-failure
```

## Benchmark

```bash
./build/lru_bench --benchmark_format=console
```

The CI pipeline prints the same numbers into the workflow summary and uploads
them as the `benchmark-report` artifact.

## Documentation

```bash
cmake --build build --target docs      # requires Doxygen
```

CI regenerates the docs and publishes them to the `gh-pages` branch on every
push to `main`.

## License

MIT — see [LICENSE](LICENSE).
