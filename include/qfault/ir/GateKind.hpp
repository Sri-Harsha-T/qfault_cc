#pragma once

namespace qfault {

enum class GateKind {
    H,    // Hadamard
    X,    // Pauli-X
    Y,    // Pauli-Y
    Z,    // Pauli-Z
    S,    // S = T²
    Sdg,  // S†
    T,    // T gate  (non-Clifford)
    Tdg,  // T†
    CX,   // CNOT (control-X)
    CZ,   // controlled-Z
    RZ,   // Rz(θ) — requires LogicalGate::angle
    U,    // U(θ,φ,λ) — requires LogicalGate::angle (θ only for now)
};

} // namespace qfault
