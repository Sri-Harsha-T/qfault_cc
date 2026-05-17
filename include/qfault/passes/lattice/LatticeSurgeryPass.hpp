#pragma once

#include <qfault/ir/MeasBasis.hpp>
#include <qfault/ir/PatchCoord.hpp>
#include <qfault/ir/PauliFrameUpdate.hpp>
#include <qfault/ir/QFaultIRModule.hpp>
#include <qfault/passes/PassBase.hpp>
#include <qfault/passes/routing/Layouts.hpp>

#include <cstddef>
#include <string_view>
#include <vector>

namespace qfault {

// Converts a LOGICAL QFaultIRModule (post-synthesis) into a PHYSICAL one by
// mapping each logical gate to surface-code PatchOps via lattice surgery.
//
// This is the only pass that transitions IRLevel::LOGICAL → IRLevel::PHYSICAL
// (ADR-0001). It must be the last pass in the pipeline before output emission.
//
// CNOT recipe (ADR-0020, Horsman et al. 2012):
//   Cycle t   (1τ): MERGE(C, A, Z-basis)  — MZZ joint measurement
//                   SPLIT(C, A)
//   Cycle t+1 (1τ): MERGE(A, T, X-basis)  — MXX joint measurement
//                   MEASURE(A, X-basis)
//   Classical PauliFrameUpdate: Z on C (power m1), X on T (power m2⊕mA)
//   Total cost: 2τ = 2d code cycles (ADR-0020; NOT d, NOT 1τ per cycle).
//
// Long-range CNOT: each individual MERGE takes 1τ regardless of chain length L.
// The total CNOT cost is still 2τ; "1τ regardless of L" refers to a single MERGE.
class LatticeSurgeryPass : public PassBase {
public:
    explicit LatticeSurgeryPass(Layout layout);

    // Non-copyable, non-movable — holds layout by value for lifetime safety.
    LatticeSurgeryPass(const LatticeSurgeryPass&)            = delete;
    LatticeSurgeryPass& operator=(const LatticeSurgeryPass&) = delete;
    LatticeSurgeryPass(LatticeSurgeryPass&&)                 = delete;
    LatticeSurgeryPass& operator=(LatticeSurgeryPass&&)      = delete;

    [[nodiscard]] std::string_view name() const override { return "LatticeSurgeryPass"; }
    [[nodiscard]] IRLevel requiredLevel() const override { return IRLevel::LOGICAL; }

    PassResult run(QFaultIRModule& module, PassContext& ctx) override;

    // ── Exposed for unit testing ──────────────────────────────────────────────

    // PatchCoord of the k-th logical qubit in the layout (0-indexed).
    // Qubits are assigned row-major by DataPatch discovery order in the PatchSpec.
    [[nodiscard]] PatchCoord qubitCoord(std::size_t qubit_index) const;

    // Emit the 4-PatchOp + 1-PauliFrameUpdate local CNOT recipe for adjacent
    // patches (3-patch L-shape). Ancilla is the empty tile between control and
    // target (midpoint in x when both are in row 0).
    // Precondition: qubitCoord(ctrl.index) and qubitCoord(tgt.index) are adjacent
    // (|Δx| == 2, Δy == 0) — asserts otherwise.
    [[nodiscard]] std::vector<Instruction> emitLocalCNOT(
        const LogicalQubit& ctrl, const LogicalQubit& tgt,
        std::size_t timeStep) const;

    // Emit a single multi-patch MERGE PatchOp along the given routing path.
    // The path includes the start and end coords plus all intermediate ancilla
    // tiles. Takes exactly 1τ regardless of path length (routing rules §long-range).
    [[nodiscard]] Instruction emitLongRangeMerge(
        const std::vector<PatchCoord>& path,
        MeasBasis basis, std::size_t timeStep) const;

    // Validate that a MERGE between two patch coords in the given basis is legal
    // (matching boundary kinds: X↔X for MeasBasis::X, Z↔Z for MeasBasis::Z).
    // Returns false on mismatch — production code asserts (logic error, ADR-0019).
    [[nodiscard]] bool validateMergeBoundaries(
        PatchCoord a, PatchCoord b, MeasBasis basis) const noexcept;

private:
    Layout                   layout_;
    std::vector<PatchCoord>  qubit_coords_; // qubit_index → PatchCoord

    // Scan layout_.spec for DataPatch tiles (row-major) and populate qubit_coords_.
    void buildQubitCoordMap(std::size_t num_qubits);
};

} // namespace qfault
