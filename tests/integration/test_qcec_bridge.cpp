// test_qcec_bridge.cpp — Issues 2.5-B-2/B-3: QCECBridge equivalence wrapper.
// Guarded by QFAULT_HAS_QCEC; GTEST_SKIP() when QCEC not linked.

#include <gtest/gtest.h>

#ifdef QFAULT_HAS_QCEC

#include <qfault/oracle/QCECBridge.hpp>

#include <string>

namespace {

// Minimal 1-qubit identity in QASM 3.0 (no gates).
const std::string kIdentity1Q = R"(
OPENQASM 3.0;
qubit[1] q;
)";

// H then H = identity (1 qubit).
const std::string kHH = R"(
OPENQASM 3.0;
qubit[1] q;
h q[0];
h q[0];
)";

// CNOT then CNOT = identity (2 qubits).
const std::string kCNOTCNOT = R"(
OPENQASM 3.0;
qubit[2] q;
cx q[0], q[1];
cx q[0], q[1];
)";

// Plain identity on 2 qubits.
const std::string kIdentity2Q = R"(
OPENQASM 3.0;
qubit[2] q;
)";

// X gate — not equivalent to identity.
const std::string kX = R"(
OPENQASM 3.0;
qubit[1] q;
x q[0];
)";

} // namespace

// ── Equivalent circuits (≤8 qubits → strict required) ────────────────────────

TEST(QCECBridge, IdenticalCircuitsAreEquivalent) {
    const auto r = qfault::oracle::check_equivalence(kHH, kHH, 1);
    EXPECT_TRUE(qfault::oracle::is_passing(r, 1))
        << "Expected passing verdict, got: " << qfault::oracle::to_string(r);
}

TEST(QCECBridge, HHEqualsIdentity) {
    const auto r = qfault::oracle::check_equivalence(kHH, kIdentity1Q, 1);
    EXPECT_TRUE(qfault::oracle::is_passing(r, 1))
        << "H;H should equal identity, got: " << qfault::oracle::to_string(r);
}

TEST(QCECBridge, CNOTCNOTEqualsIdentity) {
    const auto r = qfault::oracle::check_equivalence(kCNOTCNOT, kIdentity2Q, 2);
    EXPECT_TRUE(qfault::oracle::is_passing(r, 2))
        << "CNOT;CNOT should equal identity, got: " << qfault::oracle::to_string(r);
}

// ── Non-equivalent circuits ───────────────────────────────────────────────────

TEST(QCECBridge, XNotEquivalentToIdentity) {
    const auto r = qfault::oracle::check_equivalence(kX, kIdentity1Q, 1);
    EXPECT_FALSE(qfault::oracle::is_passing(r, 1))
        << "X != Identity; expected failing verdict, got: " << qfault::oracle::to_string(r);
}

// ── is_passing threshold dispatch ────────────────────────────────────────────

TEST(QCECBridge, StrictThresholdRequiresEquivalentFor8Qubits) {
    // For ≤8 qubits, ProbablyEquivalent must NOT pass.
    EXPECT_FALSE(qfault::oracle::is_passing(
        qfault::oracle::EquivalenceResult::ProbablyEquivalent, 8));
}

TEST(QCECBridge, RelaxedThresholdAcceptsProbablyEquivalentFor9Qubits) {
    // For >8 qubits, ProbablyEquivalent must pass.
    EXPECT_TRUE(qfault::oracle::is_passing(
        qfault::oracle::EquivalenceResult::ProbablyEquivalent, 9));
}

TEST(QCECBridge, NotEquivalentAlwaysFails) {
    EXPECT_FALSE(qfault::oracle::is_passing(
        qfault::oracle::EquivalenceResult::NotEquivalent, 4));
    EXPECT_FALSE(qfault::oracle::is_passing(
        qfault::oracle::EquivalenceResult::NotEquivalent, 10));
}

TEST(QCECBridge, ToStringReturnsExpectedLabels) {
    EXPECT_EQ(qfault::oracle::to_string(qfault::oracle::EquivalenceResult::Equivalent),
              "equivalent");
    EXPECT_EQ(qfault::oracle::to_string(qfault::oracle::EquivalenceResult::NotEquivalent),
              "not_equivalent");
    EXPECT_EQ(qfault::oracle::to_string(qfault::oracle::EquivalenceResult::ProbablyEquivalent),
              "probably_equivalent");
}

#else // !QFAULT_HAS_QCEC

TEST(QCECBridge, QcecNotLinked) {
    GTEST_SKIP() << "MQT QCEC not linked — build with -DQFAULT_ENABLE_QCEC=ON (preset: gcc13-stim)";
}

#endif // QFAULT_HAS_QCEC
