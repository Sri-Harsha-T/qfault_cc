#include <qfault/ir/GateKind.hpp>
#include <qfault/ir/IRLevel.hpp>
#include <qfault/ir/LogicalGate.hpp>
#include <qfault/ir/LogicalQubit.hpp>
#include <qfault/ir/PatchOp.hpp>
#include <qfault/ir/QFaultIRModule.hpp>

#include <gtest/gtest.h>
#include <stdexcept>
#include <variant>

using namespace qfault;

// ── LOGICAL level ──────────────────────────────────────────────────────────────

TEST(QFaultIRModule, DefaultIsLogical) {
    QFaultIRModule m;
    EXPECT_EQ(m.level, IRLevel::LOGICAL);
    EXPECT_TRUE(m.instructions.empty());
    EXPECT_TRUE(m.qubits.empty());
}

TEST(QFaultIRModule, AppendLogicalGates) {
    LogicalQubit q0{.name = "q", .index = 0};
    LogicalQubit q1{.name = "q", .index = 1};

    QFaultIRModule m{.name = "test", .level = IRLevel::LOGICAL, .qubits = {q0, q1}};
    m.instructions.push_back(LogicalGate{.kind = GateKind::H, .operands = {q0}, .angle = std::nullopt});
    m.instructions.push_back(LogicalGate{.kind = GateKind::CX, .operands = {q0, q1}, .angle = std::nullopt});
    m.instructions.push_back(LogicalGate{.kind = GateKind::T, .operands = {q1}, .angle = std::nullopt});

    EXPECT_EQ(m.instructions.size(), 3u);
    EXPECT_TRUE(std::holds_alternative<LogicalGate>(m.instructions[0]));
    EXPECT_EQ(std::get<LogicalGate>(m.instructions[0]).kind, GateKind::H);
}

TEST(QFaultIRModule, AssertLevelPassesOnCorrectLevel) {
    QFaultIRModule m{.level = IRLevel::LOGICAL};
    EXPECT_NO_THROW(m.assertLevel(IRLevel::LOGICAL));
}

TEST(QFaultIRModule, AssertLevelThrowsOnWrongLevel) {
    QFaultIRModule m{.level = IRLevel::LOGICAL};
    EXPECT_THROW(m.assertLevel(IRLevel::PHYSICAL), std::logic_error);
}

// ── PHYSICAL level ─────────────────────────────────────────────────────────────

TEST(QFaultIRModule, AppendPhysicalPatchOps) {
    QFaultIRModule m{.name = "phys", .level = IRLevel::PHYSICAL};
    m.instructions.push_back(PatchOp{
        .kind = PatchOpKind::MERGE,
        .patches = {{.x = 0, .y = 0}, {.x = 1, .y = 0}},
        .basis = MeasBasis::X,
        .timeStep = 0,
    });
    m.instructions.push_back(PatchOp{
        .kind = PatchOpKind::SPLIT,
        .patches = {{.x = 0, .y = 0}},
        .basis = MeasBasis::Z,
        .timeStep = 1,
    });

    EXPECT_EQ(m.instructions.size(), 2u);
    EXPECT_TRUE(std::holds_alternative<PatchOp>(m.instructions[0]));
    EXPECT_EQ(std::get<PatchOp>(m.instructions[0]).kind, PatchOpKind::MERGE);
}

TEST(QFaultIRModule, PhysicalAssertLevelPasses) {
    QFaultIRModule m{.level = IRLevel::PHYSICAL};
    EXPECT_NO_THROW(m.assertLevel(IRLevel::PHYSICAL));
}

TEST(QFaultIRModule, PhysicalAssertLevelThrowsOnLogical) {
    QFaultIRModule m{.level = IRLevel::PHYSICAL};
    EXPECT_THROW(m.assertLevel(IRLevel::LOGICAL), std::logic_error);
}

// ── Variant type safety ────────────────────────────────────────────────────────

TEST(QFaultIRModule, InstructionVariantHoldsCorrectType) {
    LogicalQubit q{.name = "q", .index = 0};
    LogicalGate gate{.kind = GateKind::S, .operands = {q}, .angle = std::nullopt};
    PatchOp op{.kind = PatchOpKind::IDLE, .patches = {{.x = 2, .y = 3}}};

    QFaultIRModule m{.level = IRLevel::LOGICAL};
    m.instructions.push_back(gate);

    EXPECT_TRUE(std::holds_alternative<LogicalGate>(m.instructions[0]));
    EXPECT_FALSE(std::holds_alternative<PatchOp>(m.instructions[0]));

    QFaultIRModule m2{.level = IRLevel::PHYSICAL};
    m2.instructions.push_back(op);
    EXPECT_TRUE(std::holds_alternative<PatchOp>(m2.instructions[0]));
}

// ── dump() ────────────────────────────────────────────────────────────────────

TEST(QFaultIRModule, DumpLogicalContainsGateName) {
    LogicalQubit q{.name = "q", .index = 0};
    QFaultIRModule m{.name = "mymod", .level = IRLevel::LOGICAL, .qubits = {q}};
    m.instructions.push_back(LogicalGate{.kind = GateKind::T, .operands = {q}, .angle = std::nullopt});

    std::string s = m.dumpToString();
    EXPECT_NE(s.find("LOGICAL"), std::string::npos);
    EXPECT_NE(s.find("mymod"), std::string::npos);
    EXPECT_NE(s.find("T"), std::string::npos);
    EXPECT_NE(s.find("q[0]"), std::string::npos);
}

TEST(QFaultIRModule, DumpPhysicalContainsPatchOp) {
    QFaultIRModule m{.name = "phys", .level = IRLevel::PHYSICAL};
    m.instructions.push_back(PatchOp{
        .kind = PatchOpKind::MERGE,
        .patches = {{.x = 1, .y = 2}},
        .basis = MeasBasis::Z,
        .timeStep = 5,
    });

    std::string s = m.dumpToString();
    EXPECT_NE(s.find("PHYSICAL"), std::string::npos);
    EXPECT_NE(s.find("MERGE"), std::string::npos);
    EXPECT_NE(s.find("@t=5"), std::string::npos);
}

TEST(QFaultIRModule, DumpIsReproducible) {
    LogicalQubit q{.name = "q", .index = 0};
    QFaultIRModule m{.name = "r", .level = IRLevel::LOGICAL, .qubits = {q}};
    m.instructions.push_back(LogicalGate{.kind = GateKind::H, .operands = {q}, .angle = std::nullopt});

    EXPECT_EQ(m.dumpToString(), m.dumpToString());
}
