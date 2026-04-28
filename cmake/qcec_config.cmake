# cmake/qcec_config.cmake
#
# MQT QCEC v3.5.0 equivalence checker integration via FetchContent.
# Per ADR-0009 (verification strategy).
#
# QCEC checks logical equivalence between two circuits — it complements
# Stim's stabiliser-level simulation. Together they form the "validation"
# side of ADR-0009; neither is a correctness proof (that's Stage 7).
#
# Notes:
#   - We disable Python bindings (`BUILD_MQT_QCEC_BINDINGS=OFF`) to avoid
#     pulling in pybind11 transitively at this layer.
#   - Library target name: `MQT::QCEC` (verify on bump; QCEC has historically
#     used `mqt_qcec` and `MQT::QCEC` in different versions).

include_guard(GLOBAL)

if(NOT DEFINED QFAULT_QCEC_TAG)
    message(FATAL_ERROR "qcec_config.cmake requires dependency_versions.cmake first")
endif()

include(FetchContent)

FetchContent_Declare(
    mqt_qcec
    GIT_REPOSITORY https://github.com/cda-tum/mqt-qcec.git
    GIT_TAG        ${QFAULT_QCEC_TAG}
    GIT_SHALLOW    TRUE
)

# QCEC build options
set(BUILD_MQT_QCEC_BINDINGS OFF CACHE BOOL "" FORCE)
set(BUILD_MQT_QCEC_TESTS    OFF CACHE BOOL "" FORCE)

FetchContent_MakeAvailable(mqt_qcec)

# Verify the expected exported target. Bump-time check.
if(NOT TARGET MQT::QCEC)
    if(TARGET mqt_qcec)
        message(WARNING "MQT QCEC ${QFAULT_QCEC_TAG} exported 'mqt_qcec' but not 'MQT::QCEC'. "
                        "ADR-0009 expected 'MQT::QCEC' — verify and update qcec_config.cmake.")
        add_library(MQT::QCEC ALIAS mqt_qcec)
    else()
        message(FATAL_ERROR
            "Expected target 'MQT::QCEC' from MQT QCEC ${QFAULT_QCEC_TAG} but it was not "
            "exported. QCEC may have renamed targets — check ADR-0009 and re-pin.")
    endif()
endif()

message(STATUS "MQT QCEC ${QFAULT_QCEC_TAG} target 'MQT::QCEC' configured")

# Convenience helper: link a target against QCEC.
function(qfault_link_qcec target)
    target_link_libraries(${target} PRIVATE MQT::QCEC)
    target_compile_definitions(${target} PRIVATE
        QFAULT_HAS_QCEC=1
        QFAULT_QCEC_VERSION="${QFAULT_QCEC_TAG}")
endfunction()
