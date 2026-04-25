#include <qfault/passes/synthesis/TGateSynthesisPass.hpp>
#include <qfault/passes/synthesis/SKProvider.hpp>
#include <qfault/passes/PassContext.hpp>
#include <qfault/passes/PassManager.hpp>
#include <qfault/ir/GateKind.hpp>
#include <qfault/ir/IRLevel.hpp>
#include <qfault/ir/LogicalGate.hpp>
#include <qfault/ir/LogicalQubit.hpp>
#include <qfault/ir/QFaultIRModule.hpp>

#include <gtest/gtest.h>

#include <string_view>
#include <variant>
#include <vector>

namespace {

// CliffordOnlyProvider: deterministic stub returning {H,S,H} for any angle.
// Guarantees no T/Tdg in the output — used to verify the pass removes all T gates.
struct CliffordOnlyProvider {
    [[nodiscard]] std::vector<qfault::GateKind> synthesise(double, double) {
        return {qfault::GateKind::H, qfault::GateKind::S, qfault::GateKind::H};
    }
    [[nodiscard]] std::string_view name() const noexcept { return "CliffordOnlyProvider"; }
};

qfault::QFaultIRModule buildTestModule() {
    using qfault::GateKind;
    using qfault::LogicalGate;
    using qfault::LogicalQubit;

    const LogicalQubit q0{"q0", 0};
    const LogicalQubit q1{"q1", 1};
    const LogicalQubit q2{"q2", 2};

    qfault::QFaultIRModule mod;
    mod.name  = "synthesis_roundtrip";
    mod.level = qfault::IRLevel::LOGICAL;
    mod.qubits = {q0, q1, q2};

    // 10 instructions: 3 T-gates + 7 Clifford gates
    mod.instructions = {
        LogicalGate{.kind = GateKind::H,   .operands = {q0}},
        LogicalGate{.kind = GateKind::T,   .operands = {q0}},  // T #1
        LogicalGate{.kind = GateKind::CX,  .operands = {q0, q1}},
        LogicalGate{.kind = GateKind::T,   .operands = {q1}},  // T #2
        LogicalGate{.kind = GateKind::S,   .operands = {q2}},
        LogicalGate{.kind = GateKind::Tdg, .operands = {q0}},  // Tdg #3
        LogicalGate{.kind = GateKind::Z,   .operands = {q1}},
        LogicalGate{.kind = GateKind::CX,  .operands = {q1, q2}},
        LogicalGate{.kind = GateKind::H,   .operands = {q2}},
        LogicalGate{.kind = GateKind::S,   .operands = {q0}},
    };
    return mod;
}

bool containsGate(const qfault::QFaultIRModule& mod, qfault::GateKind kind) {
    for (const auto& instr : mod.instructions) {
        const auto* g = std::get_if<qfault::LogicalGate>(&instr);
        if (g && g->kind == kind) return true;
    }
    return false;
}

} // namespace

// ── CliffordOnlyProvider integration ──────────────────────────────────────────

TEST(SynthesisRoundtrip, NoTGatesRemainAfterCliffordOnlyProvider) {
    auto mod = buildTestModule();
    qfault::PassContext ctx{5};
    qfault::PassManager pm;
    pm.add<qfault::TGateSynthesisPass<CliffordOnlyProvider>>(CliffordOnlyProvider{});

    const auto result = pm.run(mod, ctx);
    ASSERT_EQ(result, qfault::PassResult::Success);
    EXPECT_FALSE(containsGate(mod, qfault::GateKind::T));
    EXPECT_FALSE(containsGate(mod, qfault::GateKind::Tdg));
}

TEST(SynthesisRoundtrip, InstructionCountExpandedAfterCliffordOnlyProvider) {
    auto mod = buildTestModule();
    const std::size_t originalCount = mod.instructions.size(); // 10

    qfault::PassContext ctx{5};
    qfault::PassManager pm;
    pm.add<qfault::TGateSynthesisPass<CliffordOnlyProvider>>(CliffordOnlyProvider{});
    pm.run(mod, ctx);

    // 3 T/Tdg gates × 3-gate replacement = 10 - 3 + 9 = 16
    const std::size_t expected = originalCount - 3 + 9;
    EXPECT_EQ(mod.instructions.size(), expected)
        << "Expected " << expected << " instructions after T expansion";
}

TEST(SynthesisRoundtrip, NonTGatesPreservedInOrder) {
    auto mod = buildTestModule();
    qfault::PassContext ctx{5};
    qfault::PassManager pm;
    pm.add<qfault::TGateSynthesisPass<CliffordOnlyProvider>>(CliffordOnlyProvider{});
    pm.run(mod, ctx);

    // CX appears only in the original circuit (provider doesn't emit CX).
    std::size_t cxCount = 0;
    for (const auto& instr : mod.instructions) {
        const auto* g = std::get_if<qfault::LogicalGate>(&instr);
        if (g && g->kind == qfault::GateKind::CX) ++cxCount;
    }
    EXPECT_EQ(cxCount, 2u) << "Both CX gates must be preserved";
}

TEST(SynthesisRoundtrip, ModuleLevelRemainsLogical) {
    auto mod = buildTestModule();
    qfault::PassContext ctx{5};
    qfault::PassManager pm;
    pm.add<qfault::TGateSynthesisPass<CliffordOnlyProvider>>(CliffordOnlyProvider{});
    pm.run(mod, ctx);
    EXPECT_EQ(mod.level, qfault::IRLevel::LOGICAL);
}

// ── SKProvider integration ─────────────────────────────────────────────────────

TEST(SynthesisRoundtrip, SKProviderRunsWithoutError) {
    auto mod = buildTestModule();
    qfault::PassContext ctx{5};
    qfault::PassManager pm;
    pm.add<qfault::TGateSynthesisPass<qfault::SKProvider>>(qfault::SKProvider{});

    const auto result = pm.run(mod, ctx);
    EXPECT_EQ(result, qfault::PassResult::Success);
    EXPECT_EQ(mod.level, qfault::IRLevel::LOGICAL);
}

TEST(SynthesisRoundtrip, AllOutputGatesAreValidCliffordT) {
    auto mod = buildTestModule();
    qfault::PassContext ctx{5};
    qfault::PassManager pm;
    pm.add<qfault::TGateSynthesisPass<qfault::SKProvider>>(qfault::SKProvider{});
    pm.run(mod, ctx);

    for (const auto& instr : mod.instructions) {
        const auto* g = std::get_if<qfault::LogicalGate>(&instr);
        if (!g) continue;
        using qfault::GateKind;
        const bool valid = (g->kind == GateKind::H   || g->kind == GateKind::X ||
                            g->kind == GateKind::Y   || g->kind == GateKind::Z ||
                            g->kind == GateKind::S   || g->kind == GateKind::Sdg ||
                            g->kind == GateKind::T   || g->kind == GateKind::Tdg ||
                            g->kind == GateKind::CX  || g->kind == GateKind::CZ);
        EXPECT_TRUE(valid) << "Unexpected gate kind " << static_cast<int>(g->kind);
    }
}
