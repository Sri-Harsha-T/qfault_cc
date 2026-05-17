#pragma once

#include <qfault/ir/LogicalQubit.hpp>
#include <qfault/ir/Pauli.hpp>
#include <qfault/ir/PauliFrameUpdate.hpp>

#include <algorithm>
#include <cstddef>
#include <map>

namespace qfault {

// Tracks the accumulated classical Pauli frame for each logical qubit.
// Corrections are absorbed here — they are NEVER executed as physical gates
// (ADR-0020: the Pauli frame is classical, absorbed into upcoming π/8 rotations
// via GoSC Fig. 4 commutation rules).
class PauliFrameTracker {
public:
    // Absorb all corrections from a PauliFrameUpdate annotation into the frame.
    void applyUpdate(const PauliFrameUpdate& update) {
        for (const auto& c : update.corrections) {
            if (c.op == Pauli::I) continue;
            auto& cur = frame_[c.qubit.index];
            cur = pauliMultiply(cur, c.op);
        }
    }

    // Return the accumulated frame for one qubit (I if no correction pending).
    [[nodiscard]] Pauli frameFor(const LogicalQubit& qubit) const {
        const auto it = frame_.find(qubit.index);
        return (it != frame_.end()) ? it->second : Pauli::I;
    }

    // True iff all qubits have identity frame (no pending corrections).
    [[nodiscard]] bool isEmpty() const noexcept {
        return std::all_of(frame_.begin(), frame_.end(),
                           [](const auto& kv) { return kv.second == Pauli::I; });
    }

    // Clear the accumulated correction for one qubit (e.g. after absorption
    // into a subsequent π/8 rotation).
    void clear(const LogicalQubit& qubit) { frame_.erase(qubit.index); }

    // Clear all pending corrections.
    void clearAll() noexcept { frame_.clear(); }

private:
    std::map<std::size_t, Pauli> frame_; // qubit.index → accumulated Pauli
};

} // namespace qfault
