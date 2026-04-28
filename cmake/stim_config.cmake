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

FetchContent_MakeAvailable(stim)

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
    target_compile_definitions(${target} PRIVATE
        QFAULT_HAS_STIM=1
        QFAULT_STIM_VERSION="${QFAULT_STIM_TAG}")
endfunction()
