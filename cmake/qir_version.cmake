# cmake/qir_version.cmake
#
# Single source of truth for the QIR specification version that the QIR
# output backend targets. Per ADR-0005, v0.1 pins to QIR Alliance v0.1
# (base profile). Bumping this is an ADR-level decision.
#
# QIR Alliance specification reference:
#   https://github.com/qir-alliance/qir-spec
#
# Profile reference:
#   "base"     — minimal feature set; no dynamic-rotation or measurement
#                feedback. Sufficient for v0.1 fault-tolerant compilation
#                output (all dynamic decisions resolved at compile time).
#   "adaptive" — adds measurement feedback / classical control. Not
#                required for v0.1; revisit post-Stage 5.

set(QFAULT_QIR_SPEC_VERSION "0.1"
    CACHE STRING "Pinned QIR Alliance specification version (per ADR-0005)")
set(QFAULT_QIR_PROFILE "base"
    CACHE STRING "Pinned QIR profile: base | adaptive (per ADR-0005)")

# Validate the profile is one we support
set(_qfault_supported_qir_profiles "base")
if(NOT QFAULT_QIR_PROFILE IN_LIST _qfault_supported_qir_profiles)
    message(FATAL_ERROR
        "Unsupported QIR profile '${QFAULT_QIR_PROFILE}'. "
        "Supported in v0.1: ${_qfault_supported_qir_profiles}. "
        "See ADR-0005.")
endif()

# Make the values available to C++ as compile definitions.
function(qfault_qir_target_apply target)
    target_compile_definitions(${target} PUBLIC
        QFAULT_QIR_SPEC_VERSION="${QFAULT_QIR_SPEC_VERSION}"
        QFAULT_QIR_PROFILE="${QFAULT_QIR_PROFILE}")
endfunction()

message(STATUS
    "QIR backend: spec ${QFAULT_QIR_SPEC_VERSION}, profile '${QFAULT_QIR_PROFILE}' (per ADR-0005)")
