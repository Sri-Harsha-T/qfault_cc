#pragma once

// StimOracle — Clifford circuit equivalence via Stim tableau comparison.
// Compiled only when QFAULT_HAS_STIM is defined (i.e., -DQFAULT_ENABLE_STIM=ON).
// Per ADR-0009: framed as *validation*, not formal verification.
//
// Usage:
//   auto text = qfault::oracle::ir_to_stim_text(mod);   // convert IR → Stim text
//   auto eq   = qfault::oracle::circuits_clifford_equivalent(before, after);

#ifdef QFAULT_HAS_STIM

#include <qfault/ir/QFaultIRModule.hpp>

#include <expected>
#include <span>
#include <string>

// Use W=64 for cross-machine reproducibility (ADR-0009/ADR-0021).
// Stim's internal headers trigger deprecated-copy and unused-param diagnostics
// when their templates are instantiated with a non-native W; suppress them here.
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wdeprecated-copy"
#pragma GCC diagnostic ignored "-Wunused-parameter"
#include "stim.h"
#pragma GCC diagnostic pop

// W=64 must fit within the machine's natural SIMD width (≥64 always holds for x86-64).
static_assert(64 <= stim::MAX_BITWORD_WIDTH,
    "stim::MAX_BITWORD_WIDTH < 64 — W=64 reproducibility template param is invalid.");

// The SIMD width used in all qfault_oracle template instantiations.
inline constexpr std::size_t kStimW = 64;

namespace qfault::oracle {

// Translate a QFaultIRModule containing only Clifford gates into Stim circuit
// text (suitable for stim::Circuit{text}). Returns an error string if any
// non-Clifford gate (T, Tdg, RZ, U) is present.
[[nodiscard]] std::expected<std::string, std::string>
ir_to_stim_text(const QFaultIRModule& mod);

// Check whether two Clifford-only QFaultIRModules implement the same Clifford
// operation by comparing their Stim stabiliser tableaux.
// Both modules must be at IRLevel::LOGICAL with equal qubit count.
// Returns true  → equivalent (same tableau up to global phase).
// Returns false → not equivalent.
// Returns error → one of the modules contains non-Clifford gates or a qubit mismatch.
[[nodiscard]] std::expected<bool, std::string>
circuits_clifford_equivalent(const QFaultIRModule& a, const QFaultIRModule& b);

// ── has_flow oracle (ADR-0021, S3.6) ─────────────────────────────────────────

// Primary equivalence check: verify that `circuit` has all of the given
// stabilizer flows (ADR-0021).
//
// Uses `stim::sample_if_circuit_has_stabilizer_flows` with `num_samples`
// randomization shots. Each sample independently catches flow violations with
// probability ≥ 1/2, giving false-positive rate 2⁻ⁿᵘᵐˢᵃᵐᵖˡᵉˢ.
//
// Returns true  → all flows passed every sample.
// Returns false → at least one flow failed.
// Returns error → circuit or flow construction error.
[[nodiscard]] std::expected<bool, std::string>
checkHasFlow(const stim::Circuit& circuit,
             std::span<const stim::Flow<kStimW>> flows,
             std::size_t num_samples = 256);

// Backstop equivalence check: strip noise from both circuits and verify that
// they implement the same logical unitary (for measurement-free circuits) or
// have matching detector/observable structure (for measured circuits).
//
// For measurement-free circuits: compares Stim tableau representations — exact,
//   O(n²) in qubit count, deterministic.
// For measured circuits: sweep-bit sampling is deferred (#52-ext); returns
//   error with appropriate message.
//
// Per ADR-0021: always call circuit.without_noise() first. This removes
// depolarisation and flip channels so the comparison is noiseless.
[[nodiscard]] std::expected<bool, std::string>
checkDetectorMatch(const stim::Circuit& circuit_a,
                   const stim::Circuit& circuit_b,
                   std::size_t num_shots = 1024);

} // namespace qfault::oracle

#endif // QFAULT_HAS_STIM
