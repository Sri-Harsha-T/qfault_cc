#include <qfault/passes/routing/Scheduler.hpp>

#include <qfault/ir/GateKind.hpp>
#include <qfault/ir/LogicalGate.hpp>
#include <qfault/ir/PatchOp.hpp>
#include <qfault/ir/PauliFrameUpdate.hpp>
#include <qfault/ir/QFaultIRModule.hpp>

#include <gtest/gtest.h>

#include <chrono>
#include <cstddef>
#include <string>
#include <vector>

namespace qfault {
namespace {

// ── Helpers ───────────────────────────────────────────────────────────────────

LogicalQubit q(std::size_t idx) {
    return LogicalQubit{"q" + std::to_string(idx), idx};
}

Instruction cx(std::size_t ctrl, std::size_t tgt) {
    return LogicalGate{.kind = GateKind::CX, .operands = {q(ctrl), q(tgt)}};
}

Instruction h(std::size_t qubit) {
    return LogicalGate{.kind = GateKind::H, .operands = {q(qubit)}};
}

// ── Empty / trivial ───────────────────────────────────────────────────────────

TEST(EAFScheduler, EmptyInstructionsProducesEmptySlots) {
    EAFScheduler sched;
    const auto slots = sched.schedule({});
    EXPECT_TRUE(slots.empty());
    EXPECT_EQ(sched.logicalDepth(), 0u);
}

TEST(EAFScheduler, NonGateInstructionsIgnored) {
    // PatchOp and PauliFrameUpdate should be silently skipped.
    EAFScheduler sched;
    std::vector<Instruction> instrs;
    instrs.push_back(PatchOp{.kind = PatchOpKind::IDLE,
                              .patches = {{0,0}},
                              .basis   = MeasBasis::Z,
                              .timeStep = 0});
    instrs.push_back(PauliFrameUpdate{.corrections = {}, .timeStep = 0});
    const auto slots = sched.schedule(instrs);
    EXPECT_TRUE(slots.empty());
    EXPECT_EQ(sched.logicalDepth(), 0u);
}

// ── Single gate ───────────────────────────────────────────────────────────────

TEST(EAFScheduler, SingleGateProducesOneSlot) {
    EAFScheduler sched;
    std::vector<Instruction> instrs = {cx(0, 1)};
    const auto slots = sched.schedule(instrs);
    ASSERT_EQ(slots.size(), 1u);
    EXPECT_EQ(slots[0].time_step, 0u);
    ASSERT_EQ(slots[0].gate_indices.size(), 1u);
    EXPECT_EQ(slots[0].gate_indices[0], 0u); // instruction 0
    EXPECT_EQ(sched.logicalDepth(), 1u);
}

// ── Sequential gates (same qubit) ─────────────────────────────────────────────

TEST(EAFScheduler, TwoSequentialGatesOnSameQubitMakeTwoSlots) {
    // CX q0,q1 then CX q0,q2 → both touch q0 → sequential.
    EAFScheduler sched;
    std::vector<Instruction> instrs = {cx(0, 1), cx(0, 2)};
    const auto slots = sched.schedule(instrs);
    ASSERT_EQ(slots.size(), 2u);
    EXPECT_EQ(slots[0].time_step, 0u);
    EXPECT_EQ(slots[1].time_step, 1u);
    EXPECT_EQ(sched.logicalDepth(), 2u);
}

TEST(EAFScheduler, GateIndexReferencesOriginalInstructionPosition) {
    // With a non-gate instruction first, gate at position 1 should report index 1.
    EAFScheduler sched;
    std::vector<Instruction> instrs;
    instrs.push_back(PauliFrameUpdate{.corrections = {}, .timeStep = 0});
    instrs.push_back(cx(0, 1));
    const auto slots = sched.schedule(instrs);
    ASSERT_EQ(slots.size(), 1u);
    ASSERT_EQ(slots[0].gate_indices.size(), 1u);
    EXPECT_EQ(slots[0].gate_indices[0], 1u); // index into instrs, not gate list
}

// ── Parallel gates (disjoint qubits) ─────────────────────────────────────────

TEST(EAFScheduler, TwoParallelGatesOnDisjointQubitsMakeOneSlot) {
    // CX q0,q1 and CX q2,q3 are qubit-disjoint → can execute in parallel.
    EAFScheduler sched;
    std::vector<Instruction> instrs = {cx(0, 1), cx(2, 3)};
    const auto slots = sched.schedule(instrs);
    ASSERT_EQ(slots.size(), 1u);
    EXPECT_EQ(slots[0].gate_indices.size(), 2u);
    EXPECT_EQ(sched.logicalDepth(), 1u);
}

TEST(EAFScheduler, TenParallelGatesMakeOneSlot) {
    // 10 CX gates on disjoint qubit pairs → all in one slot (depth=1).
    EAFScheduler sched;
    std::vector<Instruction> instrs;
    for (std::size_t k = 0; k < 10; ++k) {
        instrs.push_back(cx(2*k, 2*k+1));
    }
    const auto slots = sched.schedule(instrs);
    ASSERT_EQ(slots.size(), 1u);
    EXPECT_EQ(slots[0].gate_indices.size(), 10u);
    EXPECT_EQ(sched.logicalDepth(), 1u);
}

// ── Diamond DAG ───────────────────────────────────────────────────────────────

TEST(EAFScheduler, DiamondDagHasDepth3) {
    // Gate A: CX q0,q1      (touches q0, q1)
    // Gate B: CX q1,q2      (touches q1, q2) → depends on A
    // Gate C: CX q0,q3      (touches q0, q3) → depends on A
    // Gate D: CX q2,q3      (touches q2, q3) → depends on B and C
    //
    // DAG:  A → B → D
    //         → C →
    // Layer 0: [A], Layer 1: [B, C], Layer 2: [D]
    EAFScheduler sched;
    std::vector<Instruction> instrs = {
        cx(0, 1), // A: idx 0
        cx(1, 2), // B: idx 1 (depends on A via q1)
        cx(0, 3), // C: idx 2 (depends on A via q0)
        cx(2, 3), // D: idx 3 (depends on B via q2, depends on C via q3)
    };
    const auto slots = sched.schedule(instrs);
    ASSERT_EQ(slots.size(), 3u);
    EXPECT_EQ(slots[0].gate_indices.size(), 1u); // A
    EXPECT_EQ(slots[1].gate_indices.size(), 2u); // B, C in parallel
    EXPECT_EQ(slots[2].gate_indices.size(), 1u); // D
    EXPECT_EQ(sched.logicalDepth(), 3u);
}

// ── Mixed gate types ──────────────────────────────────────────────────────────

TEST(EAFScheduler, SingleQubitGatesScheduledCorrectly) {
    // H q0, H q1 → qubit-disjoint → one slot
    // H q0 again → depends on first H q0 → second slot
    EAFScheduler sched;
    std::vector<Instruction> instrs = {h(0), h(1), h(0)};
    const auto slots = sched.schedule(instrs);
    ASSERT_EQ(slots.size(), 2u);
    EXPECT_EQ(slots[0].gate_indices.size(), 2u); // H(0), H(1) in parallel
    EXPECT_EQ(slots[1].gate_indices.size(), 1u); // H(0) again
}

// ── Determinism ───────────────────────────────────────────────────────────────

TEST(EAFScheduler, SameInputProducesSameOutput) {
    std::vector<Instruction> instrs = {cx(0,1), cx(2,3), cx(0,2), cx(1,3)};
    EAFScheduler s1, s2;
    const auto slots1 = s1.schedule(instrs);
    const auto slots2 = s2.schedule(instrs);
    ASSERT_EQ(slots1.size(), slots2.size());
    for (std::size_t i = 0; i < slots1.size(); ++i) {
        EXPECT_EQ(slots1[i].gate_indices, slots2[i].gate_indices);
        EXPECT_EQ(slots1[i].time_step,    slots2[i].time_step);
    }
}

// ── Logical depth ─────────────────────────────────────────────────────────────

TEST(EAFScheduler, LogicalDepthEqualsNumberOfSlots) {
    EAFScheduler sched;
    // 4 sequential gates on q0 → depth 4
    std::vector<Instruction> instrs = {h(0), h(0), h(0), h(0)};
    const auto slots = sched.schedule(instrs);
    EXPECT_EQ(slots.size(), sched.logicalDepth());
    EXPECT_EQ(sched.logicalDepth(), 4u);
}

TEST(EAFScheduler, LogicalDepthResetsBetweenCalls) {
    EAFScheduler sched;
    [[maybe_unused]] auto s1 = sched.schedule({cx(0,1), cx(0,1), cx(0,1)}); // depth 3
    EXPECT_EQ(sched.logicalDepth(), 3u);
    [[maybe_unused]] auto s2 = sched.schedule({cx(0,1)});                   // depth 1
    EXPECT_EQ(sched.logicalDepth(), 1u);
}

// ── Throughput ────────────────────────────────────────────────────────────────

TEST(EAFScheduler, ThroughputExceeds10kGatesPerSecond) {
    // Build 1000 sequential-chain gates on q0 (worst-case depth = 1000).
    // Must schedule in < 100ms (= 10k gates/sec).
    constexpr std::size_t N = 1000;
    std::vector<Instruction> instrs;
    instrs.reserve(N);
    for (std::size_t i = 0; i < N; ++i) {
        instrs.push_back(h(0));
    }

    EAFScheduler sched;
    const auto t0 = std::chrono::steady_clock::now();
    const auto slots = sched.schedule(instrs);
    const auto t1 = std::chrono::steady_clock::now();

    const auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(t1 - t0).count();
    EXPECT_LT(ms, 100LL) << "Scheduler took " << ms << "ms for " << N << " gates";
    EXPECT_EQ(slots.size(), N); // all sequential
}

} // namespace
} // namespace qfault
