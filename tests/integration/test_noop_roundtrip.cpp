#include <qfault/ir/GateKind.hpp>
#include <qfault/ir/IRLevel.hpp>
#include <qfault/ir/LogicalGate.hpp>
#include <qfault/ir/LogicalQubit.hpp>
#include <qfault/ir/PatchOp.hpp>
#include <qfault/ir/QFaultIRModule.hpp>
#include <qfault/passes/NoOpPass.hpp>
#include <qfault/passes/PassContext.hpp>
#include <qfault/passes/PassManager.hpp>

#include <gtest/gtest.h>

using namespace qfault;

TEST(NoOpRoundTrip, LogicalModuleUnchangedAfterNoOpPass) {
    LogicalQubit q0{.name = "q", .index = 0};
    LogicalQubit q1{.name = "q", .index = 1};

    QFaultIRModule m{.name = "clifford_t", .level = IRLevel::LOGICAL, .qubits = {q0, q1}};
    m.instructions.push_back(LogicalGate{.kind = GateKind::H,   .operands = {q0}});
    m.instructions.push_back(LogicalGate{.kind = GateKind::CX,  .operands = {q0, q1}});
    m.instructions.push_back(LogicalGate{.kind = GateKind::T,   .operands = {q1}});
    m.instructions.push_back(LogicalGate{.kind = GateKind::S,   .operands = {q0}});
    m.instructions.push_back(LogicalGate{.kind = GateKind::Tdg, .operands = {q1}});

    const std::string before = m.dumpToString();

    PassContext ctx{5};
    PassManager pm;
    pm.add<NoOpPass>(IRLevel::LOGICAL);
    ASSERT_EQ(pm.run(m, ctx), PassResult::Success);

    ASSERT_EQ(m.dumpToString(), before);
}

TEST(NoOpRoundTrip, PhysicalModuleUnchangedAfterNoOpPass) {
    QFaultIRModule m{.name = "surface", .level = IRLevel::PHYSICAL};
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

    const std::string before = m.dumpToString();

    PassContext ctx{7};
    PassManager pm;
    pm.add<NoOpPass>(IRLevel::PHYSICAL);
    ASSERT_EQ(pm.run(m, ctx), PassResult::Success);

    ASSERT_EQ(m.dumpToString(), before);
}

TEST(NoOpRoundTrip, NoOpPassRejectsWrongLevel) {
    QFaultIRModule m{.level = IRLevel::PHYSICAL};
    PassContext ctx{5};
    NoOpPass pass{IRLevel::LOGICAL};
    EXPECT_THROW(pass.run(m, ctx), std::logic_error);
}

TEST(NoOpRoundTrip, MultipleNoOpPassesPreserveOrder) {
    LogicalQubit q{.name = "q", .index = 0};
    QFaultIRModule m{.name = "repeat", .level = IRLevel::LOGICAL, .qubits = {q}};
    m.instructions.push_back(LogicalGate{.kind = GateKind::T, .operands = {q}});
    m.instructions.push_back(LogicalGate{.kind = GateKind::H, .operands = {q}});

    const std::string before = m.dumpToString();

    PassContext ctx{3};
    PassManager pm;
    pm.add<NoOpPass>(IRLevel::LOGICAL)
      .add<NoOpPass>(IRLevel::LOGICAL)
      .add<NoOpPass>(IRLevel::LOGICAL);

    ASSERT_EQ(pm.run(m, ctx), PassResult::Success);
    ASSERT_EQ(m.dumpToString(), before);
}
