#pragma once

// StimBackend — converts a LOGICAL QFaultIRModule to a stim::Circuit for
// correctness validation via has_flow (ADR-0021).
//
// v0.1: logical-level emission (one Stim qubit per logical qubit; no physical
// syndrome extraction rounds). Correct for Clifford-only circuits (BV-10 etc.)
// and passes has_flow / unsigned_stabilizer_flow checks.
// Physical surface code expansion (REPEAT d { ... } syndrome blocks) is
// deferred to a future issue (#51-ext).
//
// Compiled only when QFAULT_HAS_STIM is defined.

#ifdef QFAULT_HAS_STIM

#include <qfault/ir/QFaultIRModule.hpp>
#include <qfault/oracle/StimOracle.hpp> // provides kStimW + Stim headers (with pragma guards)

#include <expected>
#include <string>

namespace qfault::oracle {

// Convert a LOGICAL QFaultIRModule to a stim::Circuit.
//
// Only Clifford gates are supported (H, X, Y, Z, S, Sdg, CX, CZ).
// T/Tdg/RZ/U gates cause an error — they should not appear in a LOGICAL IR
// module that has already been through TGateSynthesisPass, or the caller
// should skip synthesis for Clifford-only circuits.
//
// An empty module (no gates) is anchored by emitting `I` on all declared
// qubits (no-op; prevents Stim from reporting 0 qubits for the empty circuit).
[[nodiscard]] std::expected<stim::Circuit, std::string>
emit_logical_stim(const QFaultIRModule& logical_module);

// Build the Z-observable Flow for logical qubit `qubit_index` in a circuit
// with `num_qubits` total qubits.
// Returns the identity flow Z[qubit_index] → Z[qubit_index] (no measurements).
// Compose with the circuit action to get the output flow for has_flow checking.
[[nodiscard]] stim::Flow<kStimW>
make_z_identity_flow(std::size_t qubit_index, std::size_t num_qubits);

} // namespace qfault::oracle

#endif // QFAULT_HAS_STIM
