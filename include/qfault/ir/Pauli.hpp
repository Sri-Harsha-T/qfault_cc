#pragma once

#include <cstdint>

namespace qfault {

// Pauli operators. Values match Stim's tableau convention (ADR-0021) and are
// load-bearing — do NOT reorder.
enum class Pauli : std::uint8_t { I = 0, X = 1, Y = 2, Z = 3 };

// Pauli multiplication (ignoring global phase).
// XOR trick: I=0, X=1, Y=2, Z=3; for non-identity inputs a≠b: a·b = a^b.
[[nodiscard]] inline Pauli pauliMultiply(Pauli a, Pauli b) noexcept {
    if (a == Pauli::I) return b;
    if (b == Pauli::I) return a;
    if (a == b)        return Pauli::I;
    return static_cast<Pauli>(static_cast<std::uint8_t>(a) ^
                               static_cast<std::uint8_t>(b));
}

} // namespace qfault
