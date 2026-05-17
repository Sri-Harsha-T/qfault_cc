#pragma once

// QCECBridge — MQT QCEC equivalence checker wrapper.
// Compiled only when QFAULT_HAS_QCEC is defined (i.e., -DQFAULT_ENABLE_QCEC=ON).
// Per ADR-0009: framed as *validation*, not formal verification.
//
// Usage:
//   auto verdict = qfault::oracle::check_equivalence(qasmA, qasmB, nqubits);

#ifdef QFAULT_HAS_QCEC

#include <cstddef>
#include <string>
#include <string_view>

namespace qfault::oracle {

// Mirrors ec::EquivalenceCriterion (re-exported to avoid including QCEC in all TUs).
enum class EquivalenceResult : unsigned char {
    NotEquivalent           = 0,
    Equivalent              = 1,
    NoInformation           = 2,
    ProbablyEquivalent      = 3,
    EquivalentUpToGlobalPhase = 4,
    EquivalentUpToPhase     = 5,
    ProbablyNotEquivalent   = 6,
};

// Convert to the string form used in golden verdict files.
[[nodiscard]] std::string_view to_string(EquivalenceResult r) noexcept;

// Check equivalence of two OpenQASM 3.0 strings.
// Checkers: Alternating + Simulation + ZX ON; Construction OFF (per ADR-0009).
// nqubits: number of logical qubits in the circuits (used for pass/fail threshold).
//   ≤ kStrictThreshold (default 8): Equivalent or EquivalentUpToGlobalPhase required.
//   > kStrictThreshold: ProbablyEquivalent accepted as pass.
[[nodiscard]] EquivalenceResult
check_equivalence(const std::string& qasmA, const std::string& qasmB,
                  std::size_t nqubits);

// Qubit count below which strict equivalence (not just probably-equivalent) is required.
inline constexpr std::size_t kQcecStrictThreshold = 8;

// True if the result is a passing verdict under the qubit-threshold policy.
[[nodiscard]] inline bool is_passing(EquivalenceResult r, std::size_t nqubits) noexcept {
    if (nqubits <= kQcecStrictThreshold) {
        return r == EquivalenceResult::Equivalent ||
               r == EquivalenceResult::EquivalentUpToGlobalPhase;
    }
    return r == EquivalenceResult::Equivalent ||
           r == EquivalenceResult::EquivalentUpToGlobalPhase ||
           r == EquivalenceResult::ProbablyEquivalent;
}

} // namespace qfault::oracle

#endif // QFAULT_HAS_QCEC
