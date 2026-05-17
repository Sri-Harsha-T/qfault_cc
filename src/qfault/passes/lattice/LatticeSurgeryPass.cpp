#include <qfault/passes/lattice/LatticeSurgeryPass.hpp>

#include <qfault/ir/GateKind.hpp>
#include <qfault/ir/PatchOpKind.hpp>
#include <qfault/passes/PassContext.hpp>
#include <qfault/passes/routing/AStar.hpp>
#include <qfault/passes/routing/Scheduler.hpp>

#include <algorithm>
#include <cassert>
#include <string>

namespace qfault {

LatticeSurgeryPass::LatticeSurgeryPass(Layout layout)
    : layout_{std::move(layout)} {
    // Pre-populate qubit_coords_ from the layout so qubitCoord() is callable
    // without first calling run() — required by unit tests and helper methods.
    const PatchSpec& spec = layout_.spec;
    std::size_t count = 0;
    for (int y = 0; y < spec.height; ++y)
        for (int x = 0; x < spec.width; ++x)
            if (spec.stateAt(x, y) == TileState::DataPatch) ++count;
    buildQubitCoordMap(count);
}

void LatticeSurgeryPass::buildQubitCoordMap(std::size_t num_qubits) {
    qubit_coords_.clear();
    qubit_coords_.reserve(num_qubits);
    const PatchSpec& spec = layout_.spec;
    for (int y = 0; y < spec.height; ++y) {
        for (int x = 0; x < spec.width; ++x) {
            if (spec.stateAt(x, y) == TileState::DataPatch) {
                qubit_coords_.push_back({x, y});
                if (qubit_coords_.size() == num_qubits) return;
            }
        }
    }
}

PatchCoord LatticeSurgeryPass::qubitCoord(std::size_t qubit_index) const {
    assert(qubit_index < qubit_coords_.size());
    return qubit_coords_[qubit_index];
}

bool LatticeSurgeryPass::validateMergeBoundaries(
    PatchCoord a, PatchCoord b, MeasBasis basis) const noexcept {

    const PatchSpec& spec = layout_.spec;
    const BoundaryKind expected =
        (basis == MeasBasis::X) ? BoundaryKind::X : BoundaryKind::Z;

    // Find the touching sides between a and b.
    // Determine which side of patch a faces patch b.
    Side side_a = Side::East;
    Side side_b = Side::West;
    if (b.x == a.x + 1) { side_a = Side::East;  side_b = Side::West;  }
    else if (b.x == a.x - 1) { side_a = Side::West;  side_b = Side::East;  }
    else if (b.y == a.y + 1) { side_a = Side::North; side_b = Side::South; }
    else if (b.y == a.y - 1) { side_a = Side::South; side_b = Side::North; }
    else { return false; } // not adjacent

    const NodeId id_a = spec.encode(a.x, a.y);
    const NodeId id_b = spec.encode(b.x, b.y);

    const auto& bnd_a = spec.boundaries[id_a][static_cast<std::size_t>(side_a)];
    const auto& bnd_b = spec.boundaries[id_b][static_cast<std::size_t>(side_b)];

    return bnd_a.has_value() && bnd_b.has_value() &&
           *bnd_a == expected && *bnd_b == expected;
}

std::vector<Instruction> LatticeSurgeryPass::emitLocalCNOT(
    const LogicalQubit& ctrl, const LogicalQubit& tgt,
    std::size_t timeStep) const {

    const PatchCoord cc = qubitCoord(ctrl.index);
    const PatchCoord tc = qubitCoord(tgt.index);

    // Ancilla is the empty tile at the midpoint between control and target.
    // Precondition: patches differ by exactly 2 in x (adjacent with 1-tile gap).
    assert(cc.y == tc.y && std::abs(cc.x - tc.x) == 2);
    const PatchCoord anc{(cc.x + tc.x) / 2, cc.y};

    // Ancilla is an empty routing tile; its boundary type is dynamically
    // established by the MERGE operation — no pre-set spec boundary to assert.

    std::vector<Instruction> out;
    out.reserve(5);

    // Cycle t (1τ): MZZ(C, A)
    out.push_back(PatchOp{
        .kind     = PatchOpKind::MERGE,
        .patches  = {cc, anc},
        .basis    = MeasBasis::Z,
        .timeStep = timeStep,
    });
    out.push_back(PatchOp{
        .kind     = PatchOpKind::SPLIT,
        .patches  = {cc, anc},
        .basis    = MeasBasis::Z,
        .timeStep = timeStep,
    });

    // Cycle t+1 (1τ): MXX(A, T) + MEASURE(A, X)
    out.push_back(PatchOp{
        .kind     = PatchOpKind::MERGE,
        .patches  = {anc, tc},
        .basis    = MeasBasis::X,
        .timeStep = timeStep + 1,
    });
    out.push_back(PatchOp{
        .kind     = PatchOpKind::MEASURE,
        .patches  = {anc},
        .basis    = MeasBasis::X,
        .timeStep = timeStep + 1,
    });

    // Classical Pauli-frame correction (ADR-0020):
    //   Z on control (conditioned on m1), X on target (conditioned on m2⊕mA).
    // Tracked classically by PauliFrameTracker — never a physical gate.
    out.push_back(PauliFrameUpdate{
        .corrections = {
            {ctrl, Pauli::Z}, // Z on control
            {tgt,  Pauli::X}, // X on target
        },
        .timeStep = timeStep + 1,
    });

    return out;
}

Instruction LatticeSurgeryPass::emitLongRangeMerge(
    const std::vector<PatchCoord>& path,
    MeasBasis basis, std::size_t timeStep) const {

    assert(!path.empty());
    // A multi-patch MERGE takes 1τ regardless of path length (routing rules).
    return PatchOp{
        .kind     = PatchOpKind::MERGE,
        .patches  = path,
        .basis    = basis,
        .timeStep = timeStep,
    };
}

PassResult LatticeSurgeryPass::run(QFaultIRModule& module, PassContext& ctx) {
    module.assertLevel(IRLevel::LOGICAL);

    buildQubitCoordMap(module.qubits.size());

    // Schedule logical gates into parallel slots (EAF, trivial commutation rule).
    EAFScheduler scheduler;
    const auto slots = scheduler.schedule(module.instructions);

    std::vector<Instruction> physical;
    physical.reserve(module.instructions.size() * 5);
    std::size_t tau     = 0; // current τ slot (code-cycle base)
    std::size_t cxCount = 0;

    for (const auto& slot : slots) {
        std::size_t slot_cost = 0; // τ consumed by this slot (max across gates)

        for (const std::size_t idx : slot.gate_indices) {
            const auto& gate = std::get<LogicalGate>(module.instructions[idx]);

            if (gate.kind == GateKind::CX) {
                assert(gate.operands.size() == 2);
                const LogicalQubit& ctrl = gate.operands[0];
                const LogicalQubit& tgt  = gate.operands[1];
                const PatchCoord cc = qubitCoord(ctrl.index);
                const PatchCoord tc = qubitCoord(tgt.index);

                if (cc.y == tc.y && std::abs(cc.x - tc.x) == 2) {
                    // Adjacent qubits — local 3-patch L-shape recipe (2τ).
                    auto instrs = emitLocalCNOT(ctrl, tgt, tau);
                    physical.insert(physical.end(), instrs.begin(), instrs.end());
                } else {
                    // Non-adjacent: IDLE placeholder until Silva EAF routing (#50-ext).
                    physical.push_back(PatchOp{
                        .kind     = PatchOpKind::IDLE,
                        .patches  = {cc},
                        .basis    = MeasBasis::Z,
                        .timeStep = tau,
                    });
                    physical.push_back(PatchOp{
                        .kind     = PatchOpKind::IDLE,
                        .patches  = {tc},
                        .basis    = MeasBasis::Z,
                        .timeStep = tau,
                    });
                    physical.push_back(PauliFrameUpdate{
                        .corrections = {{ctrl, Pauli::Z}, {tgt, Pauli::X}},
                        .timeStep    = tau + 1,
                    });
                }
                slot_cost = std::max(slot_cost, std::size_t{2});
                ++cxCount;
            } else {
                // Single-qubit or other gate: IDLE on each operand.
                for (const auto& q : gate.operands) {
                    if (q.index < qubit_coords_.size()) {
                        physical.push_back(PatchOp{
                            .kind     = PatchOpKind::IDLE,
                            .patches  = {qubitCoord(q.index)},
                            .basis    = MeasBasis::Z,
                            .timeStep = tau,
                        });
                    }
                }
                slot_cost = std::max(slot_cost, std::size_t{1});
            }
        }

        tau += slot_cost;
    }

    module.instructions = std::move(physical);
    module.level = IRLevel::PHYSICAL;

    ctx.addDiagnostic(DiagLevel::Info,
        "LatticeSurgeryPass: LOGICAL→PHYSICAL, " + std::to_string(cxCount) +
        " CX gate(s), " + std::to_string(tau) + " τ, depth=" +
        std::to_string(scheduler.logicalDepth()) + " slot(s)");

    return PassResult::Success;
}

} // namespace qfault
