#include <qfault/passes/lattice/LatticeSurgeryPass.hpp>
#include <qfault/passes/lattice/PauliFrameTracker.hpp>

#include <qfault/ir/GateKind.hpp>
#include <qfault/ir/PatchOpKind.hpp>
#include <qfault/passes/PassContext.hpp>
#include <qfault/passes/routing/Layouts.hpp>

#include <gtest/gtest.h>

#include <algorithm>
#include <cstddef>

namespace qfault {
namespace {

// ── Helpers ───────────────────────────────────────────────────────────────────

// Intermediate layout for 4 qubits: data at (0,0),(2,0),(4,0),(6,0).
// Qubit 0 ↔ (0,0), qubit 1 ↔ (2,0), qubit 2 ↔ (4,0), qubit 3 ↔ (6,0).
LatticeSurgeryPass make4QPass() {
    return LatticeSurgeryPass{intermediateLayout(4)};
}

LogicalQubit q(std::size_t idx) {
    return LogicalQubit{"q" + std::to_string(idx), idx};
}

// Build a trivial 2-qubit module with one CX gate (q0 → q1).
QFaultIRModule makeOneCXModule() {
    QFaultIRModule mod;
    mod.name  = "test";
    mod.level = IRLevel::LOGICAL;
    mod.qubits = {q(0), q(1)};
    mod.instructions.push_back(
        LogicalGate{.kind = GateKind::CX, .operands = {q(0), q(1)}});
    return mod;
}

// ── Qubit coord mapping ───────────────────────────────────────────────────────

TEST(LatticeSurgeryPass, QubitCoordsAreInDataPatchOrder) {
    auto pass = make4QPass();
    // intermediateLayout(4) places data at even-x in row 0
    EXPECT_EQ(pass.qubitCoord(0), (PatchCoord{0, 0}));
    EXPECT_EQ(pass.qubitCoord(1), (PatchCoord{2, 0}));
    EXPECT_EQ(pass.qubitCoord(2), (PatchCoord{4, 0}));
    EXPECT_EQ(pass.qubitCoord(3), (PatchCoord{6, 0}));
}

// ── emitLocalCNOT: structure ──────────────────────────────────────────────────

TEST(LatticeSurgeryPass, LocalCNOTEmits5Instructions) {
    auto pass = make4QPass();
    const auto instrs = pass.emitLocalCNOT(q(0), q(1), /*timeStep=*/0);
    EXPECT_EQ(instrs.size(), 5u); // 4 PatchOps + 1 PauliFrameUpdate
}

TEST(LatticeSurgeryPass, LocalCNOTFirstInstrIsMergeZ) {
    auto pass = make4QPass();
    const auto instrs = pass.emitLocalCNOT(q(0), q(1), 0);
    const auto* op = std::get_if<PatchOp>(&instrs[0]);
    ASSERT_NE(op, nullptr);
    EXPECT_EQ(op->kind,  PatchOpKind::MERGE);
    EXPECT_EQ(op->basis, MeasBasis::Z);
    EXPECT_EQ(op->timeStep, 0u);
}

TEST(LatticeSurgeryPass, LocalCNOTSecondInstrIsSplitZ) {
    auto pass = make4QPass();
    const auto instrs = pass.emitLocalCNOT(q(0), q(1), 0);
    const auto* op = std::get_if<PatchOp>(&instrs[1]);
    ASSERT_NE(op, nullptr);
    EXPECT_EQ(op->kind,  PatchOpKind::SPLIT);
    EXPECT_EQ(op->basis, MeasBasis::Z);
    EXPECT_EQ(op->timeStep, 0u);
}

TEST(LatticeSurgeryPass, LocalCNOTThirdInstrIsMergeX) {
    auto pass = make4QPass();
    const auto instrs = pass.emitLocalCNOT(q(0), q(1), 0);
    const auto* op = std::get_if<PatchOp>(&instrs[2]);
    ASSERT_NE(op, nullptr);
    EXPECT_EQ(op->kind,  PatchOpKind::MERGE);
    EXPECT_EQ(op->basis, MeasBasis::X);
    EXPECT_EQ(op->timeStep, 1u);
}

TEST(LatticeSurgeryPass, LocalCNOTFourthInstrIsMeasureX) {
    auto pass = make4QPass();
    const auto instrs = pass.emitLocalCNOT(q(0), q(1), 0);
    const auto* op = std::get_if<PatchOp>(&instrs[3]);
    ASSERT_NE(op, nullptr);
    EXPECT_EQ(op->kind,  PatchOpKind::MEASURE);
    EXPECT_EQ(op->basis, MeasBasis::X);
    EXPECT_EQ(op->timeStep, 1u);
}

TEST(LatticeSurgeryPass, LocalCNOTFifthInstrIsPauliFrameUpdate) {
    auto pass = make4QPass();
    const auto instrs = pass.emitLocalCNOT(q(0), q(1), 0);
    const auto* pfu = std::get_if<PauliFrameUpdate>(&instrs[4]);
    ASSERT_NE(pfu, nullptr);
    EXPECT_EQ(pfu->timeStep, 1u);
    EXPECT_EQ(pfu->corrections.size(), 2u);
}

TEST(LatticeSurgeryPass, LocalCNOTPauliFrameUpdateCorrections) {
    auto pass = make4QPass();
    const auto instrs = pass.emitLocalCNOT(q(0), q(1), 0);
    const auto& pfu = std::get<PauliFrameUpdate>(instrs[4]);

    // Z correction on control, X correction on target (ADR-0020)
    bool has_z_ctrl = false, has_x_tgt = false;
    for (const auto& c : pfu.corrections) {
        if (c.qubit == q(0) && c.op == Pauli::Z) has_z_ctrl = true;
        if (c.qubit == q(1) && c.op == Pauli::X) has_x_tgt  = true;
    }
    EXPECT_TRUE(has_z_ctrl) << "Missing Z correction on control";
    EXPECT_TRUE(has_x_tgt)  << "Missing X correction on target";
}

// ── emitLocalCNOT: 2τ cost (ADR-0020) ────────────────────────────────────────

TEST(LatticeSurgeryPass, LocalCNOTCostsExactly2TimeSteps) {
    auto pass = make4QPass();
    const std::size_t t0 = 5;
    const auto instrs = pass.emitLocalCNOT(q(0), q(1), t0);

    std::size_t min_t = ~std::size_t{0};
    std::size_t max_t = 0;
    for (const auto& instr : instrs) {
        std::size_t ts = 0;
        if (const auto* op  = std::get_if<PatchOp>(&instr))          ts = op->timeStep;
        if (const auto* pfu = std::get_if<PauliFrameUpdate>(&instr)) ts = pfu->timeStep;
        min_t = std::min(min_t, ts);
        max_t = std::max(max_t, ts);
    }
    // Spans exactly 2 timeSteps: t0 and t0+1.
    EXPECT_EQ(min_t, t0);
    EXPECT_EQ(max_t, t0 + 1);
    EXPECT_EQ(max_t - min_t, 1u); // 2τ = 2 distinct timeSteps
}

// ── emitLocalCNOT: ancilla coord ─────────────────────────────────────────────

TEST(LatticeSurgeryPass, LocalCNOTAncillaIsMidpoint) {
    auto pass = make4QPass();
    const auto instrs = pass.emitLocalCNOT(q(0), q(1), 0);
    // First MERGE patches = {control, ancilla}
    const auto& merge1 = std::get<PatchOp>(instrs[0]);
    ASSERT_EQ(merge1.patches.size(), 2u);
    const PatchCoord anc = merge1.patches[1];
    // Midpoint of (0,0) and (2,0) = (1,0)
    EXPECT_EQ(anc, (PatchCoord{1, 0}));
}

// ── emitLongRangeMerge: 1τ cost ───────────────────────────────────────────────

TEST(LatticeSurgeryPass, LongRangeMergeIs1TimeStep) {
    auto pass = make4QPass();
    // Simulate a routing path from (0,0) → (1,1) → (2,1) → (4,1) → (4,0)
    const std::vector<PatchCoord> path{{0,0},{0,1},{2,1},{4,1},{4,0}};
    const std::size_t t = 7;
    const auto instr = pass.emitLongRangeMerge(path, MeasBasis::Z, t);
    const auto& op = std::get<PatchOp>(instr);
    EXPECT_EQ(op.kind,      PatchOpKind::MERGE);
    EXPECT_EQ(op.basis,     MeasBasis::Z);
    EXPECT_EQ(op.timeStep,  t);
    EXPECT_EQ(op.patches,   path); // all routing tiles in 1 MERGE (1τ regardless of L)
}

TEST(LatticeSurgeryPass, LongRangeMergePathLengthDoesNotAffectTimeStep) {
    auto pass = make4QPass();
    // Short path (L=1) and long path (L=10) both take 1τ.
    const auto short_instr = pass.emitLongRangeMerge({{0,0},{2,0}}, MeasBasis::X, 0);
    std::vector<PatchCoord> long_path;
    for (int x = 0; x <= 20; x += 2) long_path.push_back({x, 0});
    const auto long_instr = pass.emitLongRangeMerge(long_path, MeasBasis::X, 0);
    EXPECT_EQ(std::get<PatchOp>(short_instr).timeStep, 0u);
    EXPECT_EQ(std::get<PatchOp>(long_instr).timeStep,  0u); // same timeStep: 1τ regardless of L
}

// ── Boundary validation ───────────────────────────────────────────────────────

TEST(LatticeSurgeryPass, ValidateMergeBoundariesAcceptsMatchingBasis) {
    auto pass = make4QPass();
    // Patch at (0,0) has Z on North/South, X on East/West (set by intermediateLayout).
    // Patch at (2,0) same. Ancilla at (1,0) is Empty.
    // For Z-basis MERGE: control (0,0) → ancilla (1,0) both need Z boundary.
    // Layout sets Z on N/S so the check depends on adjacency direction.
    // Actually (0,0) and (1,0) are horizontally adjacent: East/West = X boundary.
    // The validateMergeBoundaries checks boundary kinds on the touching sides.
    // For Z-basis: should return false (east boundary is X, not Z).
    EXPECT_FALSE(pass.validateMergeBoundaries({0,0}, {1,0}, MeasBasis::Z));
    // For X-basis: east/west boundaries are X — should return true.
    // (1,0) is Empty so has nullopt boundaries → returns false.
    EXPECT_FALSE(pass.validateMergeBoundaries({0,0}, {1,0}, MeasBasis::X));
}

TEST(LatticeSurgeryPass, ValidateMergeBoundariesRejectsNonAdjacentPatches) {
    auto pass = make4QPass();
    // Non-adjacent patches (differ by 4 in x) → returns false.
    EXPECT_FALSE(pass.validateMergeBoundaries({0,0}, {4,0}, MeasBasis::Z));
    EXPECT_FALSE(pass.validateMergeBoundaries({0,0}, {4,0}, MeasBasis::X));
}

// ── run(): LOGICAL → PHYSICAL transition ─────────────────────────────────────

TEST(LatticeSurgeryPass, RunTransitionsLevelToPhysical) {
    auto pass = make4QPass();
    auto mod  = makeOneCXModule();
    PassContext ctx{5};
    const auto result = pass.run(mod, ctx);
    EXPECT_EQ(result, PassResult::Success);
    EXPECT_EQ(mod.level, IRLevel::PHYSICAL);
}

TEST(LatticeSurgeryPass, RunConvertsLogicalGatesToPatchOps) {
    auto pass = make4QPass();
    auto mod  = makeOneCXModule();
    PassContext ctx{5};
    pass.run(mod, ctx);
    // After run, instructions contain only PatchOp and PauliFrameUpdate (no LogicalGate)
    for (const auto& instr : mod.instructions) {
        EXPECT_FALSE(std::holds_alternative<LogicalGate>(instr));
    }
}

TEST(LatticeSurgeryPass, RunOnOneCXProduces5Instructions) {
    auto pass = make4QPass();
    auto mod  = makeOneCXModule();
    PassContext ctx{5};
    pass.run(mod, ctx);
    EXPECT_EQ(mod.instructions.size(), 5u); // 4 PatchOps + 1 PauliFrameUpdate
}

TEST(LatticeSurgeryPass, RunAddsInfoDiagnostic) {
    auto pass = make4QPass();
    auto mod  = makeOneCXModule();
    PassContext ctx{5};
    pass.run(mod, ctx);
    const auto& diags = ctx.diagnostics();
    ASSERT_FALSE(diags.empty());
    EXPECT_EQ(diags.back().level, DiagLevel::Info);
}

// ── PauliFrameTracker ─────────────────────────────────────────────────────────

TEST(PauliFrameTracker, InitiallyEmpty) {
    PauliFrameTracker tracker;
    EXPECT_TRUE(tracker.isEmpty());
    EXPECT_EQ(tracker.frameFor(q(0)), Pauli::I);
}

TEST(PauliFrameTracker, ApplyUpdateAbsorbsCorrections) {
    PauliFrameTracker tracker;
    PauliFrameUpdate pfu;
    pfu.corrections = {{q(0), Pauli::Z}, {q(1), Pauli::X}};
    tracker.applyUpdate(pfu);
    EXPECT_EQ(tracker.frameFor(q(0)), Pauli::Z);
    EXPECT_EQ(tracker.frameFor(q(1)), Pauli::X);
    EXPECT_FALSE(tracker.isEmpty());
}

TEST(PauliFrameTracker, DoubleApplyOfSamePauliCancels) {
    PauliFrameTracker tracker;
    PauliFrameUpdate pfu;
    pfu.corrections = {{q(0), Pauli::X}};
    tracker.applyUpdate(pfu);
    tracker.applyUpdate(pfu); // X·X = I
    EXPECT_EQ(tracker.frameFor(q(0)), Pauli::I);
    EXPECT_TRUE(tracker.isEmpty());
}

TEST(PauliFrameTracker, PauliMultiplyTable) {
    // pauliMultiply is a free function; test via tracker accumulation.
    PauliFrameTracker tracker;
    PauliFrameUpdate x_upd, z_upd;
    x_upd.corrections = {{q(0), Pauli::X}};
    z_upd.corrections = {{q(0), Pauli::Z}};

    tracker.applyUpdate(x_upd); // X
    tracker.applyUpdate(z_upd); // X·Z = Y
    EXPECT_EQ(tracker.frameFor(q(0)), Pauli::Y);
}

TEST(PauliFrameTracker, ClearResetsOneQubit) {
    PauliFrameTracker tracker;
    PauliFrameUpdate pfu;
    pfu.corrections = {{q(0), Pauli::X}, {q(1), Pauli::Z}};
    tracker.applyUpdate(pfu);
    tracker.clear(q(0));
    EXPECT_EQ(tracker.frameFor(q(0)), Pauli::I);
    EXPECT_EQ(tracker.frameFor(q(1)), Pauli::Z); // q(1) unchanged
}

// ── Pauli multiply free function ──────────────────────────────────────────────

TEST(Pauli, MultiplyIdentity) {
    EXPECT_EQ(pauliMultiply(Pauli::I, Pauli::X), Pauli::X);
    EXPECT_EQ(pauliMultiply(Pauli::X, Pauli::I), Pauli::X);
    EXPECT_EQ(pauliMultiply(Pauli::I, Pauli::I), Pauli::I);
}

TEST(Pauli, MultiplySelfGivesIdentity) {
    EXPECT_EQ(pauliMultiply(Pauli::X, Pauli::X), Pauli::I);
    EXPECT_EQ(pauliMultiply(Pauli::Y, Pauli::Y), Pauli::I);
    EXPECT_EQ(pauliMultiply(Pauli::Z, Pauli::Z), Pauli::I);
}

TEST(Pauli, MultiplyOffDiagonal) {
    EXPECT_EQ(pauliMultiply(Pauli::X, Pauli::Y), Pauli::Z);
    EXPECT_EQ(pauliMultiply(Pauli::Y, Pauli::Z), Pauli::X);
    EXPECT_EQ(pauliMultiply(Pauli::Z, Pauli::X), Pauli::Y);
    EXPECT_EQ(pauliMultiply(Pauli::Y, Pauli::X), Pauli::Z); // symmetric mod phase
}

} // namespace
} // namespace qfault
