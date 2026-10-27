# AGENTS.md

Instructions for AI coding agents (Claude Code, Codex, Cursor, Copilot, …)
working in this repository. Read this file before changing anything.

## What this project is

`lru-cache` is a header-only C++20 library that implements a thread-safe LRU
cache (`lru::LruCache`), plus unit tests, micro-benchmarks and Doxygen
documentation. The repository is a teaching project: the build, test, benchmark
and documentation pipeline matters as much as the code itself.

## Repository layout

| Path | Purpose |
| --- | --- |
| `include/lru/lru_cache.hpp` | the whole library; public API lives in namespace `lru` |
| `tests/test_lru.cpp` | GoogleTest unit tests, one behaviour per `TEST` |
| `benchmarks/bench_lru.cpp` | Google Benchmark workloads |
| `docs/Doxyfile.in` | Doxygen template, substituted by CMake |
| `.github/workflows/ci.yml` | build + test + benchmark + docs pipeline |
| `CMakeLists.txt` | build definition and dependency fetching |

## Commands

```bash
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release   # configure
cmake --build build --parallel                   # build
ctest --test-dir build --output-on-failure       # run unit tests
./build/lru_bench --benchmark_format=console     # run benchmarks
cmake --build build --target docs                # generate docs (needs Doxygen)
```

## Rules for the agent

1. **Before you finish any task, build the project and run the tests.** Do not
   report success if `ctest` fails.
2. **Do not modify CI/CD without being asked.** `.github/workflows/ci.yml` is
   graded as-is; changes must be requested explicitly.
3. **Do not modify `AGENTS.md` or `CLAUDE.md` unless the task asks for it.**
4. Keep the public API in `namespace lru`; additions must be documented with
   Doxygen comments (`///` or `/** … */`).
5. Every new behaviour needs a unit test in `tests/test_lru.cpp`; every new hot
   path should be covered by a benchmark case.
6. Code style: C++20, 4 spaces, no tabs, `snake_case` for functions and
   variables, `PascalCase` for types, headers use `#pragma once`.
7. Prefer the standard library over new dependencies; any new dependency must be
   fetched through CMake `FetchContent` and pinned to a tag.
8. Do not commit build artefacts (`build/`), editor files or binaries.
9. If a task is ambiguous, state the assumption in the pull request description
   instead of silently guessing.
