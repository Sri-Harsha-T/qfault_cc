// test_stage_3_gate.cpp — Issue #54: Stage 3 gate test (BV-10 end-to-end).
//
// Falsifiable Stage 3 predicate: does QFault produce correct Stim output for
// BV-10 at d=5, satisfying has_flow for all 10 logical Z observables and
// tile-count = 2*10+4 = 24 for the intermediate layout?
//
// BV-10 phase-oracle formulation (10 logical qubits, no explicit ancilla):
//   H q0..q9 → Z q0..q9 (oracle, secret=1111111111) → H q0..q9
//   Equivalent to X on each qubit (Clifford circuit, no T gates).
//
// Step 1: Construct BV-10 logical IR directly (qasmbench submodule deferred to
//         #3.7.1; bv_n10.qasm small/ is not yet in the repo).
// Step 2: TGateSynthesis — skipped (all Clifford; no T gates in BV-10).
// Step 3: Run LatticeSurgeryPass with intermediateLayout(10) at d=5.
// Step 4: Emit logical Stim via emit_logical_stim (before LSPass, LOGICAL IR).
// Step 5: Exact golden match — deferred (#51-ext physical StimBackend).
// Step 6: checkHasFlow for 10 Z observables.
// Step 7: Physical qubit count [539, 800] — deferred (#51-ext).
// Step 8: Detector count [1000, 1250] — deferred (#51-ext).
// Step 9: Assert tile count = 2*10+4 = 24 (intermediate layout).

#ifdef QFAULT_HAS_STIM

#include <qfault/oracle/StimBackend.hpp>
#include <qfault/oracle/StimOracle.hpp>
#include <qfault/passes/lattice/LatticeSurgeryPass.hpp>
#include <qfault/passes/routing/Layouts.hpp>

#include <qfault/ir/GateKind.hpp>
#include <qfault/ir/IRLevel.hpp>
#include <qfault/ir/LogicalGate.hpp>
#include <qfault/ir/LogicalQubit.hpp>
#include <qfault/ir/QFaultIRModule.hpp>
#include <qfault/passes/PassContext.hpp>

#include <gtest/gtest.h>

#include <span>
#include <string>
#include <vector>

namespace {

constexpr std::size_t kNQubits = 10;
constexpr int         kDistance = 5;

qfault::LogicalQubit q(std::size_t idx) {
    return qfault::LogicalQubit{"q" + std::to_string(idx), idx};
}

// BV-10 phase oracle, mixed secret s = 0101010101:
//   Even qubits (0,2,4,6,8): s_i=0 → H H = I  → Z_i → +Z_i (unsigned AND signed pass)
//   Odd  qubits (1,3,5,7,9): s_i=1 → H Z H = X → Z_i → -Z_i (signed flow "Zi -> -Zi")
//
// Using a mixed secret makes the signed has_flow check non-trivial: positive flows
// pass for even qubits while negative flows are required for odd qubits.
qfault::QFaultIRModule make_bv10() {
    using namespace qfault;
    QFaultIRModule mod;
    mod.name  = "bv_n10";
    mod.level = IRLevel::LOGICAL;
    for (std::size_t i = 0; i < kNQubits; ++i) mod.qubits.push_back(q(i));

    // H layer on all qubits
    for (std::size_t i = 0; i < kNQubits; ++i)
        mod.instructions.push_back(LogicalGate{.kind = GateKind::H, .operands = {q(i)}});
    // Oracle layer: Z only on odd qubits (secret bits at positions 1,3,5,7,9 = 1)
    for (std::size_t i = 1; i < kNQubits; i += 2)
        mod.instructions.push_back(LogicalGate{.kind = GateKind::Z, .operands = {q(i)}});
    // Inverse H layer on all qubits
    for (std::size_t i = 0; i < kNQubits; ++i)
        mod.instructions.push_back(LogicalGate{.kind = GateKind::H, .operands = {q(i)}});

    return mod;
}

// Total instructions: 10 (H) + 5 (Z on odd) + 10 (H) = 25.
constexpr std::size_t kExpectedInstrCount = 25;

} // namespace

// ── Step 1 + 2 (IR construction + synthesis no-op) ───────────────────────────

TEST(Stage3Gate, BV10ModuleHasTenQubitsAndCorrectInstrCount) {
    const auto mod = make_bv10();
    EXPECT_EQ(mod.qubits.size(), kNQubits);
    EXPECT_EQ(mod.instructions.size(), kExpectedInstrCount); // 10H + 5Z + 10H = 25
    EXPECT_EQ(mod.level, qfault::IRLevel::LOGICAL);
}

// ── Step 3: LatticeSurgeryPass (tile count gate) ──────────────────────────────

TEST(Stage3Gate, IntermediateLayoutTileCountIs24) {
    // Spec predicate 9: intermediateLayout(10).tile_count == 2*10+4 = 24.
    using namespace qfault;
    const Layout layout = intermediateLayout(kNQubits);
    EXPECT_EQ(layout.tile_count, 2 * kNQubits + 4)
        << "intermediate layout tile count formula 2n+4 must equal 24 for n=10";
    EXPECT_EQ(layout.tile_count, 24u);
}

TEST(Stage3Gate, LatticeSurgeryPassRunsWithoutError) {
    using namespace qfault;
    auto mod = make_bv10();

    // Build intermediate layout at d=5.
    const Layout layout = intermediateLayout(kNQubits);
    LatticeSurgeryPass lsp{layout};

    PassContext ctx{kDistance};
    ASSERT_NO_THROW({
        lsp.run(mod, ctx);
    }) << "LatticeSurgeryPass must not throw on BV-10";

    EXPECT_EQ(mod.level, IRLevel::PHYSICAL)
        << "Pass must transition module to PHYSICAL IR";
}

// ── Step 4: Logical Stim emission ────────────────────────────────────────────

TEST(Stage3Gate, EmitLogicalStimSucceeds) {
    const auto mod = make_bv10();
    const auto result = qfault::oracle::emit_logical_stim(mod);
    ASSERT_TRUE(result.has_value()) << result.error();
    EXPECT_EQ(result->count_qubits(), kNQubits);
}

// ── Step 5: Exact golden match — deferred to #51-ext ─────────────────────────
// The physical StimBackend (syndrome extraction REPEAT blocks, detectors,
// OBSERVABLE_INCLUDE) is deferred to #51-ext. Exact golden match requires
// physical-level Stim circuit which we cannot yet produce.

TEST(Stage3Gate, ExactGoldenMatchDeferred) {
    GTEST_SKIP() << "#51-ext: physical StimBackend (REPEAT d syndrome extraction) "
                    "deferred — exact golden comparison skipped";
}

// ── Step 6: checkHasFlow for 10 logical Z observables ────────────────────────
//
// BV-10 mixed-secret phase oracle (s = 0101010101):
//   Even qubits (0,2,4,6,8): H H = I   → signed flow "Zi -> Zi"  passes
//   Odd  qubits (1,3,5,7,9): H Z H = X → signed flow "Zi -> -Zi" passes
//
// checkHasFlow uses sample_if_circuit_has_stabilizer_flows (SIGNED check).

TEST(Stage3Gate, HasFlowForAllTenZObservables) {
    using namespace qfault::oracle;
    const auto mod = make_bv10();
    const auto circuit_res = emit_logical_stim(mod);
    ASSERT_TRUE(circuit_res.has_value()) << circuit_res.error();
    const stim::Circuit& circuit = *circuit_res;

    // Build 10 Z-observable flows matching the mixed-secret BV oracle.
    std::vector<stim::Flow<kStimW>> flows;
    flows.reserve(kNQubits);
    for (std::size_t i = 0; i < kNQubits; ++i) {
        const std::string idx = std::to_string(i);
        if (i % 2 == 0) {
            // s_i=0: identity (H H = I) → Z_i → +Z_i
            flows.push_back(stim::Flow<kStimW>::from_str("Z" + idx + " -> Z" + idx));
        } else {
            // s_i=1: oracle bit set (H Z H = X) → Z_i → -Z_i
            flows.push_back(stim::Flow<kStimW>::from_str("Z" + idx + " -> -Z" + idx));
        }
    }

    const auto result = checkHasFlow(circuit, flows, 128);
    ASSERT_TRUE(result.has_value()) << result.error();
    EXPECT_TRUE(*result)
        << "BV-10 (mixed-secret phase oracle): signed Z-observable flows must pass";
}

TEST(Stage3Gate, HasFlowNegativeTestFailsWrongFlow) {
    using namespace qfault::oracle;
    const auto mod = make_bv10();
    const auto circuit_res = emit_logical_stim(mod);
    ASSERT_TRUE(circuit_res.has_value()) << circuit_res.error();
    const stim::Circuit& circuit = *circuit_res;

    // Z0 → X0 is wrong for any circuit acting independently on each qubit.
    const std::vector<stim::Flow<kStimW>> bad_flows{
        stim::Flow<kStimW>::from_str("Z0 -> X0"),
    };
    const auto result = checkHasFlow(circuit, bad_flows, 128);
    ASSERT_TRUE(result.has_value()) << result.error();
    EXPECT_FALSE(*result) << "Z0 -> X0 must fail for BV-10 phase oracle";
}

TEST(Stage3Gate, HasFlowNegativeTestFailsWrongSign) {
    using namespace qfault::oracle;
    const auto mod = make_bv10();
    const auto circuit_res = emit_logical_stim(mod);
    ASSERT_TRUE(circuit_res.has_value()) << circuit_res.error();
    const stim::Circuit& circuit = *circuit_res;

    // Z1 → +Z1 must FAIL because odd qubits get H Z H = X (signed: Z → -Z).
    const std::vector<stim::Flow<kStimW>> wrong_sign{
        stim::Flow<kStimW>::from_str("Z1 -> Z1"),
    };
    const auto result = checkHasFlow(circuit, wrong_sign, 128);
    ASSERT_TRUE(result.has_value()) << result.error();
    EXPECT_FALSE(*result) << "Z1 -> Z1 (wrong sign) must fail for odd-qubit oracle";
}

TEST(Stage3Gate, SelfEquivalenceViaCircuitsEquivalent) {
    // circuits_clifford_equivalent(bv10, bv10) must return true.
    const auto mod = make_bv10();
    const auto result = qfault::oracle::circuits_clifford_equivalent(mod, mod);
    ASSERT_TRUE(result.has_value()) << result.error();
    EXPECT_TRUE(*result);
}

// ── Step 7: Physical qubit count — deferred to #51-ext ───────────────────────

TEST(Stage3Gate, PhysicalQubitCountDeferred) {
    GTEST_SKIP() << "#51-ext: physical qubit count assertion [539, 800] deferred "
                    "until physical StimBackend generates syndrome extraction circuit";
}

// ── Step 8: Detector count — deferred to #51-ext ─────────────────────────────

TEST(Stage3Gate, DetectorCountDeferred) {
    GTEST_SKIP() << "#51-ext: detector count assertion [1000, 1250] deferred "
                    "until physical StimBackend generates syndrome extraction circuit";
}

// ── Step 9 (tightened): tile count via layout API ────────────────────────────

TEST(Stage3Gate, TileCountIntermediateLayout10Equals24) {
    using namespace qfault;
    EXPECT_EQ(intermediateTileCount(10), 24u);
    EXPECT_EQ(intermediateLayout(10).tile_count, 24u);
}

#else // !QFAULT_HAS_STIM

#include <gtest/gtest.h>
TEST(Stage3Gate, SkippedNoBuildFlag) {
    GTEST_SKIP() << "QFAULT_HAS_STIM not defined — Stage 3 gate tests skipped";
}

#endif // QFAULT_HAS_STIM
