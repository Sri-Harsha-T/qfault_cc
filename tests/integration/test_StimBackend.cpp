// test_StimBackend.cpp — Issue #51: StimBackend smoke tests.
// Validates emit_logical_stim() for 1-qubit identity, 1-qubit X, and
// 2-qubit CNOT cases via Stim's has_flow / unsigned_stabilizer_flow checks.

#ifdef QFAULT_HAS_STIM

#include <qfault/oracle/StimBackend.hpp>
#include <qfault/oracle/StimOracle.hpp>

#include <qfault/ir/GateKind.hpp>
#include <qfault/ir/IRLevel.hpp>
#include <qfault/ir/LogicalGate.hpp>
#include <qfault/ir/LogicalQubit.hpp>
#include <qfault/ir/QFaultIRModule.hpp>

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wdeprecated-copy"
#pragma GCC diagnostic ignored "-Wunused-parameter"
#include "stim/util_top/has_flow.h"
#pragma GCC diagnostic pop

#include <gtest/gtest.h>

#include <random>
#include <string>
#include <vector>

namespace qfault::oracle {
namespace {

// ── Helpers ───────────────────────────────────────────────────────────────────

LogicalQubit q(std::size_t idx) {
    return LogicalQubit{"q" + std::to_string(idx), idx};
}

QFaultIRModule makeModule(std::size_t n_qubits,
                          std::vector<LogicalGate> gates) {
    QFaultIRModule mod;
    mod.name  = "test";
    mod.level = IRLevel::LOGICAL;
    for (std::size_t i = 0; i < n_qubits; ++i) {
        mod.qubits.push_back(q(i));
    }
    for (auto& g : gates) {
        mod.instructions.push_back(g);
    }
    return mod;
}

// ── Emit: basic structure ─────────────────────────────────────────────────────

TEST(StimBackend, EmitLogicalStimReturnsCircuit) {
    auto mod = makeModule(1, {});
    auto result = emit_logical_stim(mod);
    ASSERT_TRUE(result.has_value()) << result.error();
}

TEST(StimBackend, EmitLogicalStimRejectsPhysicalIR) {
    QFaultIRModule mod;
    mod.level = IRLevel::PHYSICAL;
    auto result = emit_logical_stim(mod);
    EXPECT_FALSE(result.has_value());
    EXPECT_NE(result.error().find("PHYSICAL"), std::string::npos);
}

TEST(StimBackend, EmitLogicalStimEmptyCircuitHasCorrectQubitCount) {
    auto mod = makeModule(3, {});
    auto result = emit_logical_stim(mod);
    ASSERT_TRUE(result.has_value());
    // Anchored I gate means circuit knows about 3 qubits.
    EXPECT_EQ(result->count_qubits(), 3u);
}

TEST(StimBackend, EmitLogicalStimCXCircuit) {
    auto mod = makeModule(2, {
        LogicalGate{.kind = GateKind::CX, .operands = {q(0), q(1)}}
    });
    auto result = emit_logical_stim(mod);
    ASSERT_TRUE(result.has_value()) << result.error();
    EXPECT_EQ(result->count_qubits(), 2u);
}

TEST(StimBackend, EmitLogicalStimRejectsNonClifford) {
    auto mod = makeModule(1, {
        LogicalGate{.kind = GateKind::T, .operands = {q(0)}}
    });
    auto result = emit_logical_stim(mod);
    EXPECT_FALSE(result.has_value());
}

// ── 1-qubit identity: Z flow preserved ───────────────────────────────────────

TEST(StimBackend, IdentityCircuitPreservesZFlow) {
    // Empty circuit (identity): Z[0] → Z[0] unsigned flow should pass.
    auto mod = makeModule(1, {});
    auto circuit = emit_logical_stim(mod);
    ASSERT_TRUE(circuit.has_value());

    auto flow = stim::Flow<kStimW>::from_str("Z0 -> Z0");
    const std::vector<stim::Flow<kStimW>> flows{flow};
    const auto results = stim::check_if_circuit_has_unsigned_stabilizer_flows<kStimW>(
        *circuit, std::span<const stim::Flow<kStimW>>{flows});
    ASSERT_EQ(results.size(), 1u);
    EXPECT_TRUE(results[0]) << "Identity circuit should preserve Z[0] → Z[0]";
}

TEST(StimBackend, IdentityCircuitPreservesXFlow) {
    auto mod = makeModule(1, {});
    auto circuit = emit_logical_stim(mod);
    ASSERT_TRUE(circuit.has_value());

    auto flow = stim::Flow<kStimW>::from_str("X0 -> X0");
    const std::vector<stim::Flow<kStimW>> flows{flow};
    const auto results = stim::check_if_circuit_has_unsigned_stabilizer_flows<kStimW>(
        *circuit, std::span<const stim::Flow<kStimW>>{flows});
    ASSERT_EQ(results.size(), 1u);
    EXPECT_TRUE(results[0]);
}

// ── 1-qubit X gate ────────────────────────────────────────────────────────────

TEST(StimBackend, XGateFlipsZSignSigned) {
    // X gate: Z[0] → -Z[0] (sign flip). Signed check.
    auto mod = makeModule(1, {
        LogicalGate{.kind = GateKind::X, .operands = {q(0)}}
    });
    auto circuit = emit_logical_stim(mod);
    ASSERT_TRUE(circuit.has_value());

    std::mt19937_64 rng{42};
    // The flow Z0 -> -Z0 should hold (signed).
    auto flow_neg = stim::Flow<kStimW>::from_str("Z0 -> -Z0");
    const std::vector<stim::Flow<kStimW>> flows_neg{flow_neg};
    const auto neg_results = stim::sample_if_circuit_has_stabilizer_flows<kStimW>(
        16, rng, *circuit, std::span<const stim::Flow<kStimW>>{flows_neg});
    EXPECT_TRUE(neg_results[0]) << "X gate should map Z[0] → -Z[0]";
}

TEST(StimBackend, XGatePreservesXObservable) {
    auto mod = makeModule(1, {
        LogicalGate{.kind = GateKind::X, .operands = {q(0)}}
    });
    auto circuit = emit_logical_stim(mod);
    ASSERT_TRUE(circuit.has_value());

    auto flow = stim::Flow<kStimW>::from_str("X0 -> X0");
    const std::vector<stim::Flow<kStimW>> flows{flow};
    const auto results = stim::check_if_circuit_has_unsigned_stabilizer_flows<kStimW>(
        *circuit, std::span<const stim::Flow<kStimW>>{flows});
    EXPECT_TRUE(results[0]) << "X gate should preserve X observable";
}

// ── 2-qubit CNOT ──────────────────────────────────────────────────────────────

TEST(StimBackend, CNOTPreservesCtrlZFlow) {
    // CNOT(ctrl=0, tgt=1): Z_ctrl → Z_ctrl (control Z unchanged).
    auto mod = makeModule(2, {
        LogicalGate{.kind = GateKind::CX, .operands = {q(0), q(1)}}
    });
    auto circuit = emit_logical_stim(mod);
    ASSERT_TRUE(circuit.has_value());

    auto flow = stim::Flow<kStimW>::from_str("Z0 -> Z0");
    const std::vector<stim::Flow<kStimW>> flows{flow};
    const auto results = stim::check_if_circuit_has_unsigned_stabilizer_flows<kStimW>(
        *circuit, std::span<const stim::Flow<kStimW>>{flows});
    EXPECT_TRUE(results[0]) << "CNOT should preserve ctrl Z observable";
}

TEST(StimBackend, CNOTEntanglesTargetZFlow) {
    // CNOT(ctrl=0, tgt=1): Z_tgt → Z_ctrl ⊗ Z_tgt.
    auto mod = makeModule(2, {
        LogicalGate{.kind = GateKind::CX, .operands = {q(0), q(1)}}
    });
    auto circuit = emit_logical_stim(mod);
    ASSERT_TRUE(circuit.has_value());

    // Z1 maps to Z0*Z1 after CX.
    auto flow = stim::Flow<kStimW>::from_str("Z1 -> Z0*Z1");
    const std::vector<stim::Flow<kStimW>> flows{flow};
    const auto results = stim::check_if_circuit_has_unsigned_stabilizer_flows<kStimW>(
        *circuit, std::span<const stim::Flow<kStimW>>{flows});
    EXPECT_TRUE(results[0]) << "CNOT should map Z_tgt → Z_ctrl ⊗ Z_tgt";
}

TEST(StimBackend, CNOTEntanglesCtrlXFlow) {
    // CNOT(ctrl=0, tgt=1): X_ctrl → X_ctrl ⊗ X_tgt.
    auto mod = makeModule(2, {
        LogicalGate{.kind = GateKind::CX, .operands = {q(0), q(1)}}
    });
    auto circuit = emit_logical_stim(mod);
    ASSERT_TRUE(circuit.has_value());

    auto flow = stim::Flow<kStimW>::from_str("X0 -> X0*X1");
    const std::vector<stim::Flow<kStimW>> flows{flow};
    const auto results = stim::check_if_circuit_has_unsigned_stabilizer_flows<kStimW>(
        *circuit, std::span<const stim::Flow<kStimW>>{flows});
    EXPECT_TRUE(results[0]) << "CNOT should map X_ctrl → X_ctrl ⊗ X_tgt";
}

TEST(StimBackend, CNOTPreservesTgtXFlow) {
    // CNOT(ctrl=0, tgt=1): X_tgt → X_tgt (target X unchanged).
    auto mod = makeModule(2, {
        LogicalGate{.kind = GateKind::CX, .operands = {q(0), q(1)}}
    });
    auto circuit = emit_logical_stim(mod);
    ASSERT_TRUE(circuit.has_value());

    auto flow = stim::Flow<kStimW>::from_str("X1 -> X1");
    const std::vector<stim::Flow<kStimW>> flows{flow};
    const auto results = stim::check_if_circuit_has_unsigned_stabilizer_flows<kStimW>(
        *circuit, std::span<const stim::Flow<kStimW>>{flows});
    EXPECT_TRUE(results[0]) << "CNOT should preserve tgt X observable";
}

// ── H gate (used in BV-10) ────────────────────────────────────────────────────

TEST(StimBackend, HGateSWapsXandZObservables) {
    // H maps Z → X and X → Z.
    auto mod = makeModule(1, {
        LogicalGate{.kind = GateKind::H, .operands = {q(0)}}
    });
    auto circuit = emit_logical_stim(mod);
    ASSERT_TRUE(circuit.has_value());

    auto flow_zx = stim::Flow<kStimW>::from_str("Z0 -> X0");
    auto flow_xz = stim::Flow<kStimW>::from_str("X0 -> Z0");
    const std::vector<stim::Flow<kStimW>> flows{flow_zx, flow_xz};
    const auto results = stim::check_if_circuit_has_unsigned_stabilizer_flows<kStimW>(
        *circuit, std::span<const stim::Flow<kStimW>>{flows});
    EXPECT_TRUE(results[0]) << "H should map Z → X";
    EXPECT_TRUE(results[1]) << "H should map X → Z";
}

// ── make_z_identity_flow helper ───────────────────────────────────────────────

TEST(StimBackend, MakeZIdentityFlowPassesForIdentityCircuit) {
    auto mod = makeModule(2, {});
    auto circuit = emit_logical_stim(mod);
    ASSERT_TRUE(circuit.has_value());

    // Both Z[0] → Z[0] and Z[1] → Z[1] should pass for identity.
    auto f0 = make_z_identity_flow(0, 2);
    auto f1 = make_z_identity_flow(1, 2);
    const std::vector<stim::Flow<kStimW>> flows{f0, f1};
    const auto results = stim::check_if_circuit_has_unsigned_stabilizer_flows<kStimW>(
        *circuit, std::span<const stim::Flow<kStimW>>{flows});
    EXPECT_TRUE(results[0]);
    EXPECT_TRUE(results[1]);
}

} // namespace
} // namespace qfault::oracle

#else // QFAULT_HAS_STIM not defined

#include <gtest/gtest.h>
TEST(StimBackend, SkippedNoBuildFlag) {
    GTEST_SKIP() << "QFAULT_HAS_STIM not defined — StimBackend tests skipped";
}

#endif // QFAULT_HAS_STIM
