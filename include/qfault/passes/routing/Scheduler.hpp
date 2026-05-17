#pragma once

#include <qfault/ir/QFaultIRModule.hpp>

#include <cstddef>
#include <vector>

namespace qfault {

// One group of logical gates that can execute simultaneously.
// All gates in a slot are qubit-disjoint (trivial commutation rule).
struct ScheduleSlot {
    std::vector<std::size_t> gate_indices; // indices into original instructions vector
    std::size_t              time_step{0}; // τ slot index (not code cycles)
};

// EAF dependency-graph scheduler (Silva et al. 2024, Algorithms 1+2 subset).
//
// Algorithm 1 — build DAG:
//   Two gates commute iff they act on disjoint qubit sets (trivial rule).
//   Edge A→B if B uses any qubit last written by A.
//
// Algorithm 2 — topological layers (Kahn's algorithm):
//   All nodes with in-degree 0 form one ScheduleSlot. After removing them,
//   repeat. This gives the shortest possible schedule depth.
//
// Throughput: O(n + E) where E ≤ n² (dependency edges). For BV-10 (~45 gates)
// this runs in microseconds. Throughput target: ≥ 10k gates / second.
//
// Full forest-packing (routing tile conflict resolution) is deferred to #50-ext;
// qubit-disjoint gates are already tile-disjoint for adjacent patches in the
// intermediate layout.
class EAFScheduler {
public:
    EAFScheduler() = default;

    // Compute schedule from the LOGICAL instructions.
    // Only LogicalGate instructions are scheduled; PatchOp and PauliFrameUpdate
    // variants are ignored.  gate_indices in each slot reference positions in
    // `instructions`.
    [[nodiscard]] std::vector<ScheduleSlot>
    schedule(const std::vector<Instruction>& instructions);

    // Total number of τ slots after the last schedule() call (= critical-path depth).
    [[nodiscard]] std::size_t logicalDepth() const noexcept;

private:
    std::size_t depth_{0};

    // Build predecessor list for each gate position.
    // preds[pos] = list of positions (in gate_positions) that must finish first.
    // May contain duplicates (same predecessor via multiple shared qubits) —
    // caller deduplicates before computing in-degrees.
    [[nodiscard]] static std::vector<std::vector<std::size_t>>
    buildDepGraph(const std::vector<std::size_t>& gate_positions,
                  const std::vector<Instruction>& instructions);
};

} // namespace qfault
