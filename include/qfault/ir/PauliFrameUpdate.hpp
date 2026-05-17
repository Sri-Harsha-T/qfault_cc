#pragma once

#include <qfault/ir/LogicalQubit.hpp>
#include <qfault/ir/Pauli.hpp>

#include <cstddef>
#include <vector>

namespace qfault {

// Classical Pauli-frame correction annotation emitted after a lattice surgery
// measurement sequence. Absorbed by PauliFrameTracker — never executed as a
// physical gate (ADR-0020).
//
// In the full implementation each correction is conditional on measurement
// outcomes (m1, m2, mA from the CNOT recipe). This skeleton records the
// always-present corrections; runtime conditioning is handled by the tracker.
struct PauliFrameUpdate {
    struct Correction {
        LogicalQubit qubit;
        Pauli        op{Pauli::I}; // X, Y, or Z — I is a no-op, omit it

        [[nodiscard]] bool operator==(const Correction&) const noexcept = default;
    };

    std::vector<Correction> corrections;
    std::size_t             timeStep{0};

    [[nodiscard]] bool operator==(const PauliFrameUpdate&) const noexcept = default;
};

} // namespace qfault
