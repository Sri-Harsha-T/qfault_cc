// Stage gate for ADR-0001: verify LOGICAL and PHYSICAL IR levels coexist in
// a single QFaultIRModule without structural incompatibility.
#include <qfault/ir/GateKind.hpp>
#include <qfault/ir/IRLevel.hpp>
#include <qfault/ir/LogicalGate.hpp>
#include <qfault/ir/LogicalQubit.hpp>
#include <qfault/ir/PatchOp.hpp>
#include <qfault/ir/QFaultIRModule.hpp>

#include <gtest/gtest.h>
#include <stdexcept>
#include <string>
#include <variant>

using namespace qfault;

// ── Stage gate: single module type hosts both IR levels ───────────────────────

TEST(TwoLevelIR, LogicalModuleAssertsPasses) {
    LogicalQubit q0{.name = "q", .index = 0};
    LogicalQubit q1{.name = "q", .index = 1};
    LogicalQubit q2{.name = "q", .index = 2};

    QFaultIRModule m{.name = "bell_circuit", .level = IRLevel::LOGICAL, .qubits = {q0, q1, q2}};
    m.instructions.push_back(LogicalGate{.kind = GateKind::H,  .operands = {q0}});
    m.instructions.push_back(LogicalGate{.kind = GateKind::CX, .operands = {q0, q1}});
    m.instructions.push_back(LogicalGate{.kind = GateKind::T,  .operands = {q2}});

    ASSERT_EQ(m.instructions.size(), 3u);
    EXPECT_NO_THROW(m.assertLevel(IRLevel::LOGICAL));
    EXPECT_THROW(m.assertLevel(IRLevel::PHYSICAL), std::logic_error);

    // All instructions hold LogicalGate — no PatchOp leaks in
    for (const auto& instr : m.instructions) {
        EXPECT_TRUE(std::holds_alternative<LogicalGate>(instr));
    }
}

TEST(TwoLevelIR, PhysicalModuleAssertsPasses) {
    QFaultIRModule m{.name = "bell_physical", .level = IRLevel::PHYSICAL};
    m.instructions.push_back(PatchOp{
        .kind = PatchOpKind::MERGE,
        .patches = {{.x = 0, .y = 0}, {.x = 1, .y = 0}},
        .basis = MeasBasis::X,
        .timeStep = 0,
    });
    m.instructions.push_back(PatchOp{
        .kind = PatchOpKind::MEASURE,
        .patches = {{.x = 0, .y = 0}},
        .basis = MeasBasis::Z,
        .timeStep = 1,
    });
    m.instructions.push_back(PatchOp{
        .kind = PatchOpKind::SPLIT,
        .patches = {{.x = 1, .y = 0}},
        .basis = MeasBasis::X,
        .timeStep = 2,
    });

    ASSERT_EQ(m.instructions.size(), 3u);
    EXPECT_NO_THROW(m.assertLevel(IRLevel::PHYSICAL));
    EXPECT_THROW(m.assertLevel(IRLevel::LOGICAL), std::logic_error);

    // All instructions hold PatchOp — no LogicalGate leaks in
    for (const auto& instr : m.instructions) {
        EXPECT_TRUE(std::holds_alternative<PatchOp>(instr));
    }
}

TEST(TwoLevelIR, BothLevelsInSameVariantType) {
    // Demonstrates that Instruction = variant<LogicalGate, PatchOp> accepts both
    // without any conversion or intermediate type.
    LogicalQubit q{.name = "q", .index = 0};
    Instruction logical = LogicalGate{.kind = GateKind::H, .operands = {q}};
    Instruction physical = PatchOp{.kind = PatchOpKind::IDLE, .patches = {{.x = 0, .y = 0}}};

    EXPECT_TRUE(std::holds_alternative<LogicalGate>(logical));
    EXPECT_TRUE(std::holds_alternative<PatchOp>(physical));
    EXPECT_FALSE(std::holds_alternative<PatchOp>(logical));
    EXPECT_FALSE(std::holds_alternative<LogicalGate>(physical));
}

TEST(TwoLevelIR, DumpWorksOnBothLevels) {
    LogicalQubit q{.name = "q", .index = 0};
    QFaultIRModule logical{.name = "lmod", .level = IRLevel::LOGICAL, .qubits = {q}};
    logical.instructions.push_back(LogicalGate{.kind = GateKind::T, .operands = {q}});

    QFaultIRModule physical{.name = "pmod", .level = IRLevel::PHYSICAL};
    physical.instructions.push_back(PatchOp{
        .kind = PatchOpKind::MERGE,
        .patches = {{.x = 0, .y = 0}},
        .basis = MeasBasis::Z,
        .timeStep = 3,
    });

    std::string ls = logical.dumpToString();
    std::string ps = physical.dumpToString();

    EXPECT_NE(ls.find("LOGICAL"), std::string::npos);
    EXPECT_NE(ls.find("T"), std::string::npos);

    EXPECT_NE(ps.find("PHYSICAL"), std::string::npos);
    EXPECT_NE(ps.find("MERGE"), std::string::npos);
    EXPECT_NE(ps.find("@t=3"), std::string::npos);
}

TEST(TwoLevelIR, LevelTransitionSimulation) {
    // Simulates what LatticeSurgeryPass will do: mutate level from LOGICAL to PHYSICAL
    // in place, replacing all instructions with PatchOps.
    LogicalQubit q{.name = "q", .index = 0};
    QFaultIRModule m{.name = "sim", .level = IRLevel::LOGICAL, .qubits = {q}};
    m.instructions.push_back(LogicalGate{.kind = GateKind::H, .operands = {q}});

    EXPECT_NO_THROW(m.assertLevel(IRLevel::LOGICAL));

    // Simulate lowering: replace all instructions with patch ops and flip level
    m.instructions.clear();
    m.instructions.push_back(PatchOp{
        .kind = PatchOpKind::MERGE,
        .patches = {{.x = 0, .y = 0}, {.x = 1, .y = 0}},
        .basis = MeasBasis::X,
        .timeStep = 0,
    });
    m.level = IRLevel::PHYSICAL;

    EXPECT_NO_THROW(m.assertLevel(IRLevel::PHYSICAL));
    EXPECT_THROW(m.assertLevel(IRLevel::LOGICAL), std::logic_error);
    EXPECT_TRUE(std::holds_alternative<PatchOp>(m.instructions[0]));
}
