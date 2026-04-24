#include <qfault/frontend/Parser.hpp>
#include <qfault/ir/GateKind.hpp>
#include <qfault/ir/IRLevel.hpp>
#include <qfault/ir/LogicalGate.hpp>

#include <gtest/gtest.h>
#include <variant>

using namespace qfault;
using namespace qfault::frontend;

static const char* k5q10g = R"(
OPENQASM 3.0;
qubit[5] q;
h q[0];
cx q[0], q[1];
t q[2];
s q[3];
tdg q[4];
sdg q[0];
x q[1];
y q[2];
z q[3];
cx q[4], q[0];
)";

TEST(Parser, ParsesQASMHeader) {
    auto [mod, diag] = Parser::parse("OPENQASM 3.0; qubit[1] q; h q[0];");
    EXPECT_EQ(mod.level, IRLevel::LOGICAL);
    EXPECT_TRUE(diag.empty());
}

TEST(Parser, ParsesQubitDeclaration) {
    auto [mod, diag] = Parser::parse("qubit[3] reg;");
    ASSERT_EQ(mod.qubits.size(), 3u);
    EXPECT_EQ(mod.qubits[0].name, "reg");
    EXPECT_EQ(mod.qubits[0].index, 0u);
    EXPECT_EQ(mod.qubits[2].index, 2u);
}

TEST(Parser, Parse5QubitCircuit) {
    auto [mod, diag] = Parser::parse(k5q10g);
    EXPECT_EQ(mod.qubits.size(), 5u);
    EXPECT_EQ(mod.instructions.size(), 10u);
    EXPECT_TRUE(diag.empty());
}

TEST(Parser, GateKindsCorrect) {
    auto [mod, diag] = Parser::parse(k5q10g);
    ASSERT_GE(mod.instructions.size(), 3u);
    EXPECT_EQ(std::get<LogicalGate>(mod.instructions[0]).kind, GateKind::H);
    EXPECT_EQ(std::get<LogicalGate>(mod.instructions[1]).kind, GateKind::CX);
    EXPECT_EQ(std::get<LogicalGate>(mod.instructions[2]).kind, GateKind::T);
}

TEST(Parser, OperandQubitNames) {
    auto [mod, diag] = Parser::parse("qubit[2] q; h q[0]; cx q[0], q[1];");
    const auto& hGate = std::get<LogicalGate>(mod.instructions[0]);
    ASSERT_EQ(hGate.operands.size(), 1u);
    EXPECT_EQ(hGate.operands[0].name, "q");
    EXPECT_EQ(hGate.operands[0].index, 0u);

    const auto& cxGate = std::get<LogicalGate>(mod.instructions[1]);
    ASSERT_EQ(cxGate.operands.size(), 2u);
    EXPECT_EQ(cxGate.operands[1].index, 1u);
}

TEST(Parser, UnsupportedGateProducesWarn) {
    auto [mod, diag] = Parser::parse("qubit[1] q; measure q[0];");
    EXPECT_TRUE(mod.instructions.empty());
    ASSERT_FALSE(diag.empty());
    EXPECT_EQ(diag[0].level, DiagLevel::Warn);
}

TEST(Parser, SyntaxErrorProducesErrorDiag) {
    // Missing '[' after qubit
    auto [mod, diag] = Parser::parse("qubit 2 q;");
    ASSERT_FALSE(diag.empty());
    EXPECT_EQ(diag[0].level, DiagLevel::Error);
}

TEST(Parser, ErrorRecoveryParsesNextStatement) {
    // First qubit decl is broken; second should parse fine
    auto [mod, diag] = Parser::parse("qubit q; qubit[1] good; h good[0];");
    // "good" register should be created and H should parse
    EXPECT_FALSE(mod.instructions.empty());
    EXPECT_EQ(std::get<LogicalGate>(mod.instructions[0]).kind, GateKind::H);
}

TEST(Parser, EmptySourceProducesEmptyModule) {
    auto [mod, diag] = Parser::parse("");
    EXPECT_TRUE(mod.qubits.empty());
    EXPECT_TRUE(mod.instructions.empty());
    EXPECT_TRUE(diag.empty());
}

TEST(Parser, AllCliffordTGates) {
    const char* src = "qubit[2] q; h q[0]; x q[0]; y q[0]; z q[0]; s q[0]; sdg q[0]; t q[0]; tdg q[0]; cx q[0],q[1]; cz q[0],q[1];";
    auto [mod, diag] = Parser::parse(src);
    EXPECT_EQ(mod.instructions.size(), 10u);
    EXPECT_TRUE(diag.empty());
}
