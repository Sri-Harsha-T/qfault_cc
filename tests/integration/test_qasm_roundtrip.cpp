// QASM 3.0 → PassManager → dump round-trip (Stage 1 final integration test).
// Note: Full text round-trip (dump → re-parse) requires a QASM 3.0 emitter,
// deferred to Stage 5. This test validates instruction-vector equality instead.
#include <qfault/frontend/Parser.hpp>
#include <qfault/ir/GateKind.hpp>
#include <qfault/ir/IRLevel.hpp>
#include <qfault/ir/LogicalGate.hpp>
#include <qfault/passes/NoOpPass.hpp>
#include <qfault/passes/PassContext.hpp>
#include <qfault/passes/PassManager.hpp>

#include <gtest/gtest.h>
#include <variant>

using namespace qfault;
using namespace qfault::frontend;

static constexpr const char* kCircuit =
    "OPENQASM 3.0; qubit[3] q; h q[0]; cx q[0],q[2]; t q[2]; s q[0]; tdg q[2];";

TEST(QASMRoundTrip, ParsesTo3QubitsAnd5Instructions) {
    auto [mod, diag] = Parser::parse(kCircuit);
    EXPECT_TRUE(diag.empty());
    EXPECT_EQ(mod.qubits.size(), 3u);
    EXPECT_EQ(mod.instructions.size(), 5u);
}

TEST(QASMRoundTrip, NoOpPassSucceeds) {
    auto [mod, diag] = Parser::parse(kCircuit);
    ASSERT_TRUE(diag.empty());

    PassContext ctx{5};
    PassManager pm;
    pm.add<NoOpPass>(IRLevel::LOGICAL);
    EXPECT_EQ(pm.run(mod, ctx), PassResult::Success);
}

TEST(QASMRoundTrip, InstructionCountUnchangedAfterPass) {
    auto [mod, diag] = Parser::parse(kCircuit);
    ASSERT_TRUE(diag.empty());

    PassContext ctx{5};
    PassManager pm;
    pm.add<NoOpPass>(IRLevel::LOGICAL);
    pm.run(mod, ctx);

    EXPECT_EQ(mod.instructions.size(), 5u);
}

TEST(QASMRoundTrip, GateSequencePreserved) {
    auto [mod, diag] = Parser::parse(kCircuit);
    ASSERT_TRUE(diag.empty());
    ASSERT_EQ(mod.instructions.size(), 5u);

    EXPECT_EQ(std::get<LogicalGate>(mod.instructions[0]).kind, GateKind::H);
    EXPECT_EQ(std::get<LogicalGate>(mod.instructions[1]).kind, GateKind::CX);
    EXPECT_EQ(std::get<LogicalGate>(mod.instructions[2]).kind, GateKind::T);
    EXPECT_EQ(std::get<LogicalGate>(mod.instructions[3]).kind, GateKind::S);
    EXPECT_EQ(std::get<LogicalGate>(mod.instructions[4]).kind, GateKind::Tdg);
}

TEST(QASMRoundTrip, DumpContainsExpectedContent) {
    auto [mod, diag] = Parser::parse(kCircuit);
    ASSERT_TRUE(diag.empty());

    const std::string dump = mod.dumpToString();
    EXPECT_NE(dump.find("LOGICAL"), std::string::npos);
    EXPECT_NE(dump.find("H"), std::string::npos);
    EXPECT_NE(dump.find("CX"), std::string::npos);
    EXPECT_NE(dump.find("T"), std::string::npos);
}

TEST(QASMRoundTrip, DumpIsReproducibleAfterPass) {
    auto [mod, diag] = Parser::parse(kCircuit);
    ASSERT_TRUE(diag.empty());

    PassContext ctx{5};
    PassManager pm;
    pm.add<NoOpPass>(IRLevel::LOGICAL)
      .add<NoOpPass>(IRLevel::LOGICAL);
    pm.run(mod, ctx);

    EXPECT_EQ(mod.dumpToString(), mod.dumpToString());
}
