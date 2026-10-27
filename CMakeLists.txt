cmake_minimum_required(VERSION 3.20)

project(
  lru_cache
  VERSION 1.0.0
  DESCRIPTION "A small thread-safe LRU cache with tests, benchmarks and docs"
  LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
  set(CMAKE_BUILD_TYPE Release CACHE STRING "Build type" FORCE)
endif()

option(LRU_BUILD_TESTS "Build unit tests" ON)
option(LRU_BUILD_BENCHMARKS "Build benchmarks" ON)
option(LRU_BUILD_DOCS "Add a docs target when Doxygen is available" ON)

# Header-only library target.
add_library(lru INTERFACE)
target_include_directories(lru INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_compile_features(lru INTERFACE cxx_std_20)

include(FetchContent)

if(LRU_BUILD_TESTS)
  FetchContent_Declare(
    googletest
    URL https://github.com/google/googletest/archive/refs/tags/v1.14.0.zip
    DOWNLOAD_EXTRACT_TIMESTAMP TRUE)
  set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
  FetchContent_MakeAvailable(googletest)

  enable_testing()
  add_executable(lru_tests tests/test_lru.cpp)
  target_link_libraries(lru_tests PRIVATE lru GTest::gtest_main)
  # A single CTest entry instead of gtest_discover_tests(): discovery has to
  # spawn the freshly built binary, which is fragile on Windows and in CI
  # sandboxes. gtest still reports per-test results in its own output.
  add_test(NAME lru_tests COMMAND lru_tests)
endif()

if(LRU_BUILD_BENCHMARKS)
  FetchContent_Declare(
    benchmark
    URL https://github.com/google/benchmark/archive/refs/tags/v1.8.3.zip
    DOWNLOAD_EXTRACT_TIMESTAMP TRUE)
  set(BENCHMARK_ENABLE_TESTING OFF CACHE BOOL "" FORCE)
  set(BENCHMARK_ENABLE_GTEST_TESTS OFF CACHE BOOL "" FORCE)
  FetchContent_MakeAvailable(benchmark)

  add_executable(lru_bench benchmarks/bench_lru.cpp)
  target_link_libraries(lru_bench PRIVATE lru benchmark::benchmark)
endif()

if(LRU_BUILD_DOCS)
  find_package(Doxygen QUIET)
  if(DOXYGEN_FOUND)
    set(LRU_SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR})
    set(DOXYGEN_IN ${CMAKE_CURRENT_SOURCE_DIR}/docs/Doxyfile.in)
    set(DOXYGEN_OUT ${CMAKE_CURRENT_BINARY_DIR}/Doxyfile)
    configure_file(${DOXYGEN_IN} ${DOXYGEN_OUT} @ONLY)
    add_custom_target(
      docs
      COMMAND ${DOXYGEN_EXECUTABLE} ${DOXYGEN_OUT}
      WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
      COMMENT "Generating API documentation with Doxygen"
      VERBATIM)
  endif()
endif()
