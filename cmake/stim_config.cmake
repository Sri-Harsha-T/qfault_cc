# cmake/stim_config.cmake
#
# Stim v1.15.0 simulation oracle integration via FetchContent.
# Per ADR-0009 (verification strategy: Stim + MQT QCEC framed as validation).
#
# Notes:
#   - Library target is `libstim` (NOT `stim`). Linking against `stim` will
#     fail at the configure step.
#   - `SIMD_WIDTH=64` is mandatory for cross-machine reproducibility — Stim's
#     default of 256 produces machine-specific results.
#   - Header is included as `#include "stim.h"` (NOT `stim/stim.h`).
#
# Dependency version pinned in cmake/dependency_versions.cmake.

include_guard(GLOBAL)

if(NOT DEFINED QFAULT_STIM_TAG)
    message(FATAL_ERROR "stim_config.cmake requires dependency_versions.cmake first")
endif()

include(FetchContent)

FetchContent_Declare(
    stim
    GIT_REPOSITORY https://github.com/quantumlib/Stim.git
    GIT_TAG        ${QFAULT_STIM_TAG}
    GIT_SHALLOW    TRUE
)

# Stim build options
set(SIMD_WIDTH ${QFAULT_STIM_SIMD_WIDTH} CACHE STRING "" FORCE)

# Prevent Stim from building its Python bindings (pybind11 may be a system
# package, triggering the bindings build; their warnings break under -Werror).
set(Python_FOUND FALSE)

FetchContent_MakeAvailable(stim)

# CMAKE_CXX_FLAGS is applied globally at generation time — clearing it before
# FetchContent_MakeAvailable does not isolate Stim. Instead, add -Wno-error
# to every Stim target after the fact; in GCC/Clang, a later flag overrides
# an earlier one, so this cancels the inherited -Werror without touching ours.
foreach(_stim_target IN ITEMS libstim stim stim_perf stim_python_bindings)
    if(TARGET ${_stim_target})
        target_compile_options(${_stim_target} PRIVATE -Wno-error)
        # Only libstim is needed; exclude CLI, perf harness, and Python bindings
        # from the default build (EXCLUDE_FROM_ALL). stim_perf has a link error
        # in v1.15.0 with C++23; stim_python_bindings pulls nanobind.
        if(NOT _stim_target STREQUAL "libstim")
            set_target_properties(${_stim_target} PROPERTIES EXCLUDE_FROM_ALL TRUE)
        endif()
    endif()
endforeach()

# Confirm the library target name (Stim has historically had a few)
if(NOT TARGET libstim)
    message(FATAL_ERROR
        "Expected target 'libstim' from Stim ${QFAULT_STIM_TAG} but it was not "
        "exported. Stim may have renamed targets — check ADR-0009 and re-pin.")
endif()

message(STATUS "Stim ${QFAULT_STIM_TAG} target 'libstim' configured (SIMD_WIDTH=${QFAULT_STIM_SIMD_WIDTH})")

# Convenience helper: link a target against Stim with the right include dirs.
function(qfault_link_stim target)
    target_link_libraries(${target} PRIVATE libstim)
    # SIMD_WIDTH must be passed to our TUs too (stim.h reads it to set MAX_BITWORD_WIDTH).
    target_compile_definitions(${target} PRIVATE
        QFAULT_HAS_STIM=1
        QFAULT_STIM_VERSION="${QFAULT_STIM_TAG}"
        QFAULT_STIM_SIMD_WIDTH=${QFAULT_STIM_SIMD_WIDTH}
        SIMD_WIDTH=${QFAULT_STIM_SIMD_WIDTH})
    # Stim v1.15.0 has deprecated-copy patterns inside its templates that fire
    # when instantiated in our code. Suppress at the target level (not globally).
    target_compile_options(${target} PRIVATE -Wno-deprecated-copy)
endfunction()
