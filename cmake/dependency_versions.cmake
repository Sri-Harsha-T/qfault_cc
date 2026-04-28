# cmake/dependency_versions.cmake
#
# Single source of truth for all third-party dependency versions
# (per ADR-0017 reproducibility infrastructure). Every FetchContent or
# find_package call elsewhere in the build reads from this file.
#
# Bumping any value here requires:
#   1. An ADR or a CHANGELOG entry justifying the bump
#   2. Re-running `./scripts/full-build.sh` and `./scripts/asan-ubsan.sh`
#   3. Updating Dockerfile and flake.nix in lockstep

# ── Test infrastructure ────────────────────────────────────────────────────────
set(QFAULT_GTEST_TAG          "v1.14.0"
    CACHE STRING "GoogleTest git tag")
set(QFAULT_BENCHMARK_TAG      "v1.8.3"
    CACHE STRING "Google Benchmark git tag")

# ── Stim simulation oracle (Stage 2.5) ─────────────────────────────────────────
# v1.15.0 "Terror of the Tag" (May 2025).
# Library target name is `libstim` (NOT `stim`).
# `SIMD_WIDTH=64` for cross-machine reproducibility.
set(QFAULT_STIM_TAG           "v1.15.0"
    CACHE STRING "Stim git tag (per ADR-0009)")
set(QFAULT_STIM_SIMD_WIDTH    "64"
    CACHE STRING "Stim SIMD_WIDTH (must be 64 for reproducibility)")

# ── MQT QCEC equivalence checker (Stage 2.5) ───────────────────────────────────
# v3.5.0 (Feb 2026). FetchContent C++ source with bindings disabled.
set(QFAULT_QCEC_TAG           "v3.5.0"
    CACHE STRING "MQT QCEC git tag (per ADR-0009)")

# ── Python bindings (Stage 5) ──────────────────────────────────────────────────
# v2.11.1 pinned (per ADR-0012). Revisit nanobind for v0.2.
set(QFAULT_PYBIND11_TAG       "v2.11.1"
    CACHE STRING "pybind11 git tag (per ADR-0012)")

# ── Compiler version floors ────────────────────────────────────────────────────
set(QFAULT_GCC_MIN_VERSION    "13"
    CACHE STRING "Minimum gcc version")
set(QFAULT_CLANG_MIN_VERSION  "18"
    CACHE STRING "Minimum clang version")

# ── CMake itself ───────────────────────────────────────────────────────────────
# Set in the root CMakeLists.txt cmake_minimum_required; documented here for
# completeness. Bumping requires updating CMakeLists.txt as well.
set(QFAULT_CMAKE_MIN_VERSION  "3.21"
    CACHE STRING "Minimum CMake version (presets feature)")

# ── QIR (Stage 5) — see cmake/qir_version.cmake for spec/profile ───────────────

# Print summary at configure time so the user can see what's pinned.
message(STATUS "")
message(STATUS "──── QFault dependency versions (per ADR-0017) ────")
message(STATUS "  GoogleTest      : ${QFAULT_GTEST_TAG}")
message(STATUS "  Google Benchmark: ${QFAULT_BENCHMARK_TAG}")
message(STATUS "  Stim            : ${QFAULT_STIM_TAG}     (SIMD_WIDTH=${QFAULT_STIM_SIMD_WIDTH})")
message(STATUS "  MQT QCEC        : ${QFAULT_QCEC_TAG}")
message(STATUS "  pybind11        : ${QFAULT_PYBIND11_TAG}")
message(STATUS "  gcc minimum     : ${QFAULT_GCC_MIN_VERSION}")
message(STATUS "  clang minimum   : ${QFAULT_CLANG_MIN_VERSION}")
message(STATUS "───────────────────────────────────────────────────")
message(STATUS "")
