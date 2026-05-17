// test_StimOracle_flow.cpp — Issue #52: checkHasFlow + checkDetectorMatch.
// Self-equivalence and negative tests for the has_flow oracle wrappers.

#ifdef QFAULT_HAS_STIM

#include <qfault/oracle/StimOracle.hpp>

#include <gtest/gtest.h>

#include <span>
#include <string>
#include <vector>

namespace qfault::oracle {
namespace {

// ── Helpers ───────────────────────────────────────────────────────────────────

stim::Circuit from_text(std::string_view text) {
    return stim::Circuit{text};
}

stim::Flow<kStimW> flow(std::string_view s) {
    return stim::Flow<kStimW>::from_str(std::string{s});
}

// ── checkHasFlow: identity circuit ───────────────────────────────────────────

TEST(CheckHasFlow, EmptyFlowListReturnsTrue) {
    const auto circuit = from_text("H 0\n");
    const std::vector<stim::Flow<kStimW>> flows;
    const auto result = checkHasFlow(circuit, flows);
    ASSERT_TRUE(result.has_value()) << result.error();
    EXPECT_TRUE(*result);
}

TEST(CheckHasFlow, IdentityCircuitPassesZFlow) {
    // I(q0): Z0 → Z0 should hold.
    const auto circuit = from_text("I 0\n");
    const std::vector<stim::Flow<kStimW>> flows{flow("Z0 -> Z0")};
    const auto result = checkHasFlow(circuit, flows, 64);
    ASSERT_TRUE(result.has_value()) << result.error();
    EXPECT_TRUE(*result);
}

TEST(CheckHasFlow, IdentityCircuitPassesXFlow) {
    const auto circuit = from_text("I 0\n");
    const std::vector<stim::Flow<kStimW>> flows{flow("X0 -> X0")};
    const auto result = checkHasFlow(circuit, flows, 64);
    ASSERT_TRUE(result.has_value()) << result.error();
    EXPECT_TRUE(*result);
}

// ── checkHasFlow: X gate ─────────────────────────────────────────────────────

TEST(CheckHasFlow, XGatePassesSignedNegZFlow) {
    // X: Z → -Z (sign flip); signed flow "Z0 -> -Z0" should pass.
    const auto circuit = from_text("X 0\n");
    const std::vector<stim::Flow<kStimW>> flows{flow("Z0 -> -Z0")};
    const auto result = checkHasFlow(circuit, flows, 128);
    ASSERT_TRUE(result.has_value()) << result.error();
    EXPECT_TRUE(*result);
}

TEST(CheckHasFlow, XGateFailsPositiveZFlow) {
    // X: Z → -Z, so unsigned Z→Z should fail (wrong sign but unsigned checks
    // ignore sign — actually unsigned flows IGNORE sign, so Z0→Z0 will pass).
    // Instead test: "Z0 -> X0" is definitely wrong for X gate.
    const auto circuit = from_text("X 0\n");
    const std::vector<stim::Flow<kStimW>> flows{flow("Z0 -> X0")};
    const auto result = checkHasFlow(circuit, flows, 128);
    ASSERT_TRUE(result.has_value()) << result.error();
    EXPECT_FALSE(*result);
}

TEST(CheckHasFlow, XGatePreservesXObservable) {
    // X: X → X.
    const auto circuit = from_text("X 0\n");
    const std::vector<stim::Flow<kStimW>> flows{flow("X0 -> X0")};
    const auto result = checkHasFlow(circuit, flows, 64);
    ASSERT_TRUE(result.has_value()) << result.error();
    EXPECT_TRUE(*result);
}

// ── checkHasFlow: H gate ─────────────────────────────────────────────────────

TEST(CheckHasFlow, HGateSwapsXandZFlows) {
    const auto circuit = from_text("H 0\n");
    const std::vector<stim::Flow<kStimW>> flows{
        flow("Z0 -> X0"),
        flow("X0 -> Z0"),
    };
    const auto result = checkHasFlow(circuit, flows, 64);
    ASSERT_TRUE(result.has_value()) << result.error();
    EXPECT_TRUE(*result);
}

// ── checkHasFlow: CNOT ───────────────────────────────────────────────────────

TEST(CheckHasFlow, CNOTPassesCtrlZFlow) {
    const auto circuit = from_text("CNOT 0 1\n");
    const std::vector<stim::Flow<kStimW>> flows{flow("Z0 -> Z0")};
    const auto result = checkHasFlow(circuit, flows, 64);
    ASSERT_TRUE(result.has_value()) << result.error();
    EXPECT_TRUE(*result);
}

TEST(CheckHasFlow, CNOTPassesTargetZSpreadFlow) {
    // CNOT: Z1 → Z0*Z1.
    const auto circuit = from_text("CNOT 0 1\n");
    const std::vector<stim::Flow<kStimW>> flows{flow("Z1 -> Z0*Z1")};
    const auto result = checkHasFlow(circuit, flows, 64);
    ASSERT_TRUE(result.has_value()) << result.error();
    EXPECT_TRUE(*result);
}

TEST(CheckHasFlow, CNOTFailsIncorrectFlow) {
    // Z0 does NOT map to Z1 after CNOT.
    const auto circuit = from_text("CNOT 0 1\n");
    const std::vector<stim::Flow<kStimW>> flows{flow("Z0 -> Z1")};
    const auto result = checkHasFlow(circuit, flows, 64);
    ASSERT_TRUE(result.has_value()) << result.error();
    EXPECT_FALSE(*result);
}

// ── checkHasFlow: multiple flows — all must pass ──────────────────────────────

TEST(CheckHasFlow, AllFlowsMustPassForTrueResult) {
    // H gate: both Z→X and X→Z hold.
    const auto circuit = from_text("H 0\n");
    const std::vector<stim::Flow<kStimW>> flows{
        flow("Z0 -> X0"),
        flow("X0 -> Z0"),
    };
    const auto result = checkHasFlow(circuit, flows, 64);
    ASSERT_TRUE(result.has_value()) << result.error();
    EXPECT_TRUE(*result);
}

TEST(CheckHasFlow, OneBadFlowInListReturnsFalse) {
    // Mix one correct and one incorrect flow — must return false.
    const auto circuit = from_text("H 0\n");
    const std::vector<stim::Flow<kStimW>> flows{
        flow("Z0 -> X0"),  // correct for H
        flow("Z0 -> Z0"),  // wrong for H
    };
    const auto result = checkHasFlow(circuit, flows, 64);
    ASSERT_TRUE(result.has_value()) << result.error();
    EXPECT_FALSE(*result);
}

// ── checkDetectorMatch: basic equivalences ────────────────────────────────────

TEST(CheckDetectorMatch, CircuitEquivalentToItself) {
    const auto circuit = from_text("H 0\nCNOT 0 1\n");
    const auto result = checkDetectorMatch(circuit, circuit);
    ASSERT_TRUE(result.has_value()) << result.error();
    EXPECT_TRUE(*result);
}

TEST(CheckDetectorMatch, HHEqualsIdentity) {
    // H;H is the identity — matches I on qubit 0.
    const auto hh = from_text("H 0\nH 0\n");
    const auto id = from_text("I 0\n");
    const auto result = checkDetectorMatch(hh, id);
    ASSERT_TRUE(result.has_value()) << result.error();
    EXPECT_TRUE(*result);
}

TEST(CheckDetectorMatch, XCircuitDoesNotMatchIdentity) {
    const auto x  = from_text("X 0\n");
    const auto id = from_text("I 0\n");
    const auto result = checkDetectorMatch(x, id);
    ASSERT_TRUE(result.has_value()) << result.error();
    EXPECT_FALSE(*result);
}

TEST(CheckDetectorMatch, NoisyCircuitsMatchAfterNoiseStripping) {
    // H with depolarising noise vs clean H: noiseless comparison should match.
    const auto noisy = from_text("H 0\nDEPOLARIZE1(0.001) 0\n");
    const auto clean = from_text("H 0\n");
    const auto result = checkDetectorMatch(noisy, clean);
    ASSERT_TRUE(result.has_value()) << result.error();
    EXPECT_TRUE(*result);
}

TEST(CheckDetectorMatch, MeasuredCircuitReturnsError) {
    // Measured circuits are deferred to #52-ext.
    const auto meas = from_text("H 0\nM 0\n");
    const auto id   = from_text("I 0\n");
    const auto result = checkDetectorMatch(meas, id);
    EXPECT_FALSE(result.has_value());
    EXPECT_NE(result.error().find("deferred"), std::string::npos);
}

} // namespace
} // namespace qfault::oracle

#else // QFAULT_HAS_STIM not defined

#include <gtest/gtest.h>
TEST(StimOracleFlow, SkippedNoBuildFlag) {
    GTEST_SKIP() << "QFAULT_HAS_STIM not defined — StimOracle flow tests skipped";
}

#endif // QFAULT_HAS_STIM
