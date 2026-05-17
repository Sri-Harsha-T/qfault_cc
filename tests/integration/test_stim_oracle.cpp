// test_stim_oracle.cpp — Issue 2.5-A-2: StimOracle has_flow equivalence for Clifford segments.
// Tests guarded by QFAULT_HAS_STIM; GTEST_SKIP() when Stim not linked.

#include <gtest/gtest.h>

#ifdef QFAULT_HAS_STIM

#include <qfault/oracle/StimOracle.hpp>
#include <qfault/ir/GateKind.hpp>
#include <qfault/ir/IRLevel.hpp>
#include <qfault/ir/LogicalGate.hpp>
#include <qfault/ir/LogicalQubit.hpp>
#include <qfault/ir/QFaultIRModule.hpp>

#include <string>

namespace {

// 3-qubit GHZ preparation: H q0; CNOT q0 q1; CNOT q1 q2
qfault::QFaultIRModule make_ghz() {
    using namespace qfault;
    const LogicalQubit q0{"q0", 0}, q1{"q1", 1}, q2{"q2", 2};
    QFaultIRModule mod;
    mod.name         = "ghz";
    mod.level        = IRLevel::LOGICAL;
    mod.qubits       = {q0, q1, q2};
    mod.instructions = {
        LogicalGate{.kind = GateKind::H,  .operands = {q0}},
        LogicalGate{.kind = GateKind::CX, .operands = {q0, q1}},
        LogicalGate{.kind = GateKind::CX, .operands = {q1, q2}},
    };
    return mod;
}

// 1-qubit identity: H then H (self-inverse)
qfault::QFaultIRModule make_h_h() {
    using namespace qfault;
    const LogicalQubit q{"q", 0};
    QFaultIRModule mod;
    mod.name         = "h_h";
    mod.level        = IRLevel::LOGICAL;
    mod.qubits       = {q};
    mod.instructions = {
        LogicalGate{.kind = GateKind::H, .operands = {q}},
        LogicalGate{.kind = GateKind::H, .operands = {q}},
    };
    return mod;
}

// 1-qubit identity: empty circuit (no gates)
qfault::QFaultIRModule make_identity_1q() {
    using namespace qfault;
    const LogicalQubit q{"q", 0};
    QFaultIRModule mod;
    mod.name   = "identity_1q";
    mod.level  = IRLevel::LOGICAL;
    mod.qubits = {q};
    return mod;
}

// 1-qubit circuit: just X
qfault::QFaultIRModule make_x() {
    using namespace qfault;
    const LogicalQubit q{"q", 0};
    QFaultIRModule mod;
    mod.name         = "x";
    mod.level        = IRLevel::LOGICAL;
    mod.qubits       = {q};
    mod.instructions = {LogicalGate{.kind = GateKind::X, .operands = {q}}};
    return mod;
}

} // namespace

// ── ir_to_stim_text ──────────────────────────────────────────────────────────

TEST(StimOracle, IrToStimTextGhz) {
    const auto result = qfault::oracle::ir_to_stim_text(make_ghz());
    ASSERT_TRUE(result.has_value()) << result.error();
    EXPECT_NE(result->find("H 0"),      std::string::npos);
    EXPECT_NE(result->find("CNOT 0 1"), std::string::npos);
    EXPECT_NE(result->find("CNOT 1 2"), std::string::npos);
}

TEST(StimOracle, IrToStimTextEmptyModule) {
    const auto result = qfault::oracle::ir_to_stim_text(make_identity_1q());
    ASSERT_TRUE(result.has_value()) << result.error();
    EXPECT_TRUE(result->empty());
}

TEST(StimOracle, IrToStimTextNonCliffordReturnsError) {
    using namespace qfault;
    const LogicalQubit q{"q", 0};
    QFaultIRModule mod;
    mod.name         = "non_clifford";
    mod.level        = IRLevel::LOGICAL;
    mod.qubits       = {q};
    mod.instructions = {LogicalGate{.kind = GateKind::T, .operands = {q}}};
    const auto result = qfault::oracle::ir_to_stim_text(mod);
    EXPECT_FALSE(result.has_value());
}

// ── circuits_clifford_equivalent ─────────────────────────────────────────────

TEST(StimOracle, CircuitEquivalentToItself) {
    const auto ghz = make_ghz();
    const auto result = qfault::oracle::circuits_clifford_equivalent(ghz, ghz);
    ASSERT_TRUE(result.has_value()) << result.error();
    EXPECT_TRUE(*result);
}

TEST(StimOracle, HHEqualsIdentity) {
    // H;H is the 1-qubit identity — should be equivalent to an empty circuit.
    const auto result =
        qfault::oracle::circuits_clifford_equivalent(make_h_h(), make_identity_1q());
    ASSERT_TRUE(result.has_value()) << result.error();
    EXPECT_TRUE(*result);
}

TEST(StimOracle, DifferentCircuitsAreNotEquivalent) {
    // X ≠ identity
    const auto result =
        qfault::oracle::circuits_clifford_equivalent(make_x(), make_identity_1q());
    ASSERT_TRUE(result.has_value()) << result.error();
    EXPECT_FALSE(*result);
}

TEST(StimOracle, QubitCountMismatchReturnsError) {
    const auto result =
        qfault::oracle::circuits_clifford_equivalent(make_ghz(), make_identity_1q());
    EXPECT_FALSE(result.has_value());
}

#else // !QFAULT_HAS_STIM

TEST(StimOracle, StimNotLinked) {
    GTEST_SKIP() << "Stim not linked — build with -DQFAULT_ENABLE_STIM=ON (preset: gcc13-stim)";
}

#endif // QFAULT_HAS_STIM
