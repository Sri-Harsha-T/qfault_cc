cmake_minimum_required(VERSION 3.21)
project(qfault VERSION 0.1.0 LANGUAGES CXX)

# ── Language standard ──────────────────────────────────────────────────────────
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# ── Export compile commands for clang-tidy ─────────────────────────────────────
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# ── Options ────────────────────────────────────────────────────────────────────
option(QFAULT_ENABLE_ASAN      "Enable AddressSanitizer + UBSanitizer" OFF)
option(QFAULT_ENABLE_UBSAN     "Enable UndefinedBehaviourSanitizer"    OFF)
option(QFAULT_RUN_BENCHMARKS   "Build Google Benchmark targets"        OFF)
option(QFAULT_ENABLE_PYTHON    "Build pybind11 Python bindings"        OFF)
set(QFAULT_GRIDSYNTH_PATH "" CACHE STRING "Path to GridSynth binary or library")

# ── Compiler flags ─────────────────────────────────────────────────────────────
add_compile_options(-Wall -Wextra -Wpedantic)

if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    add_compile_options(-g -O0)
elseif(CMAKE_BUILD_TYPE STREQUAL "Release")
    add_compile_options(-O3 -DNDEBUG)
endif()

if(QFAULT_ENABLE_ASAN OR QFAULT_ENABLE_UBSAN)
    add_compile_options(-fsanitize=address,undefined -fno-omit-frame-pointer -g)
    add_link_options(-fsanitize=address,undefined)
endif()

# -Werror only in CI (set via CMAKE_CXX_FLAGS in the preset, not here, to avoid
# breaking developer builds on compiler version differences)

# ── Dependencies via FetchContent ──────────────────────────────────────────────
include(FetchContent)

# Pin FetchContent downloads inside the binary directory, never the source tree
set(FETCHCONTENT_BASE_DIR "${CMAKE_BINARY_DIR}/_deps")

FetchContent_Declare(
    googletest
    GIT_REPOSITORY https://github.com/google/googletest.git
    GIT_TAG        v1.14.0
    GIT_SHALLOW    TRUE
)
# Prevent GoogleTest from overriding our compiler/linker settings
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)

if(QFAULT_RUN_BENCHMARKS)
    FetchContent_Declare(
        benchmark
        GIT_REPOSITORY https://github.com/google/benchmark.git
        GIT_TAG        v1.8.3
        GIT_SHALLOW    TRUE
    )
    set(BENCHMARK_ENABLE_TESTING OFF CACHE BOOL "" FORCE)
    FetchContent_MakeAvailable(benchmark)
endif()

# ── Library: libqfault ─────────────────────────────────────────────────────────
# Re-run cmake configure when new .cpp files are added (GLOB_RECURSE is evaluated
# at configure time, not build time).
file(GLOB_RECURSE QFAULT_SOURCES CONFIGURE_DEPENDS "src/qfault/**/*.cpp")

add_library(libqfault STATIC ${QFAULT_SOURCES})
set_target_properties(libqfault PROPERTIES OUTPUT_NAME qfault)

target_include_directories(libqfault
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:include>
    PRIVATE
        ${CMAKE_SOURCE_DIR}/src
)

target_compile_features(libqfault PUBLIC cxx_std_20)

# ── Test binary: qfault_tests ──────────────────────────────────────────────────
enable_testing()

file(GLOB_RECURSE QFAULT_TEST_SOURCES
    "tests/unit/*.cpp"
    "tests/integration/*.cpp"
)

if(QFAULT_TEST_SOURCES)
    add_executable(qfault_tests ${QFAULT_TEST_SOURCES})
    target_link_libraries(qfault_tests PRIVATE libqfault GTest::gtest_main)
    target_include_directories(qfault_tests PRIVATE ${CMAKE_SOURCE_DIR}/include)

    include(GoogleTest)
    gtest_discover_tests(qfault_tests)
else()
    # No test sources yet — add a placeholder so ctest exits 0
    add_executable(qfault_tests tests/unit/placeholder_test.cpp)
    target_link_libraries(qfault_tests PRIVATE GTest::gtest_main)
    include(GoogleTest)
    gtest_discover_tests(qfault_tests)
endif()

# ── Benchmark binary (optional) ────────────────────────────────────────────────
if(QFAULT_RUN_BENCHMARKS)
    file(GLOB_RECURSE QFAULT_BENCH_SOURCES "tests/benchmarks/*.cpp")
    if(QFAULT_BENCH_SOURCES)
        add_executable(qfault_benchmarks ${QFAULT_BENCH_SOURCES})
        target_link_libraries(qfault_benchmarks PRIVATE libqfault benchmark::benchmark_main)
        target_include_directories(qfault_benchmarks PRIVATE ${CMAKE_SOURCE_DIR}/include)
    endif()
endif()

# ── Python bindings (Stage 5) ──────────────────────────────────────────────────
if(QFAULT_ENABLE_PYTHON)
    FetchContent_Declare(
        pybind11
        GIT_REPOSITORY https://github.com/pybind/pybind11.git
        GIT_TAG        v2.11.1
        GIT_SHALLOW    TRUE
    )
    FetchContent_MakeAvailable(pybind11)
    add_subdirectory(bindings/python)
endif()

# ── Convenience targets ────────────────────────────────────────────────────────
find_program(CLANG_TIDY_EXE NAMES clang-tidy-18 clang-tidy-17 clang-tidy)
if(CLANG_TIDY_EXE)
    add_custom_target(tidy
        COMMAND ${CLANG_TIDY_EXE}
            --config-file=${CMAKE_SOURCE_DIR}/.clang-tidy
            -p ${CMAKE_BINARY_DIR}
            ${QFAULT_SOURCES}
        COMMENT "Running clang-tidy on libqfault sources"
        WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
    )
else()
    message(STATUS "clang-tidy not found — 'make tidy' target unavailable")
endif()

find_program(CLANG_FORMAT_EXE NAMES clang-format-18 clang-format-17 clang-format)
if(CLANG_FORMAT_EXE)
    file(GLOB_RECURSE ALL_CXX_FILES
        "src/**/*.cpp" "src/**/*.hpp"
        "include/**/*.hpp"
        "tests/**/*.cpp"
    )
    add_custom_target(format
        COMMAND ${CLANG_FORMAT_EXE} -i ${ALL_CXX_FILES}
        COMMENT "Running clang-format on all C++ files"
        WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
    )
else()
    message(STATUS "clang-format not found — 'make format' target unavailable")
endif()
