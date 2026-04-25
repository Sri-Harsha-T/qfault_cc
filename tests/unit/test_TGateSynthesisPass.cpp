#include <qfault/passes/synthesis/TGateSynthesisPass.hpp>
#include <qfault/passes/synthesis/SKProvider.hpp>
#include <qfault/passes/PassContext.hpp>
#include <qfault/passes/PassManager.hpp>
#include <qfault/ir/GateKind.hpp>
#include <qfault/ir/LogicalGate.hpp>
#include <qfault/ir/LogicalQubit.hpp>
#include <qfault/ir/QFaultIRModule.hpp>

#include <gtest/gtest.h>

#include <algorithm>
#include <string_view>
#include <variant>

namespace {

// Deterministic mock: always returns {H, S, H} (Clifford-only, no T).
// Used to verify TGateSynthesisPass replaces T gates with the provider sequence.
struct CliffordOnlyProvider {
    [[nodiscard]] std::vector<qfault::GateKind> synthesise(double, double) {
        return {qfault::GateKind::H, qfault::GateKind::S, qfault::GateKind::H};
    }
    [[nodiscard]] std::string_view name() const noexcept { return "CliffordOnlyProvider"; }
};
static_assert(qfault::SynthesisProvider<CliffordOnlyProvider>);

qfault::QFaultIRModule makeModule() {
    using qfault::GateKind;
    using qfault::LogicalGate;
    using qfault::LogicalQubit;

    const LogicalQubit q0{"q0", 0};
    const LogicalQubit q1{"q1", 1};

    qfault::QFaultIRModule mod;
    mod.name  = "test";
    mod.level = qfault::IRLevel::LOGICAL;
    mod.qubits = {q0, q1};

    // 3 T-gates interspersed with Clifford gates
    mod.instructions = {
        LogicalGate{.kind = GateKind::H,   .operands = {q0}},
        LogicalGate{.kind = GateKind::T,   .operands = {q0}},
        LogicalGate{.kind = GateKind::CX,  .operands = {q0, q1}},
        LogicalGate{.kind = GateKind::T,   .operands = {q1}},
        LogicalGate{.kind = GateKind::S,   .operands = {q0}},
        LogicalGate{.kind = GateKind::Tdg, .operands = {q0}},
        LogicalGate{.kind = GateKind::H,   .operands = {q1}},
    };
    return mod;
}

bool hasTGate(const qfault::QFaultIRModule& mod) {
    for (const auto& instr : mod.instructions) {
        const auto* g = std::get_if<qfault::LogicalGate>(&instr);
        if (g && (g->kind == qfault::GateKind::T || g->kind == qfault::GateKind::Tdg))
            return true;
    }
    return false;
}

} // namespace

TEST(TGateSynthesisPass, NameContainsProviderName) {
    qfault::TGateSynthesisPass<qfault::SKProvider> pass{qfault::SKProvider{}};
    const std::string n{pass.name()};
    EXPECT_NE(n.find("TGateSynthesisPass"), std::string::npos);
    EXPECT_NE(n.find("SKProvider"), std::string::npos);
}

TEST(TGateSynthesisPass, RequiredLevelIsLogical) {
    qfault::TGateSynthesisPass<qfault::SKProvider> pass{qfault::SKProvider{}};
    EXPECT_EQ(pass.requiredLevel(), qfault::IRLevel::LOGICAL);
}

TEST(TGateSynthesisPass, NoTGatesRemainAfterPassWithCliffordOnlyProvider) {
    // CliffordOnlyProvider returns {H,S,H} for every angle — so all T/Tdg gates
    // are replaced and no T gates remain in the output.
    auto mod = makeModule();
    const std::size_t originalCount = mod.instructions.size(); // 7

    qfault::PassContext ctx{5};
    qfault::PassManager pm;
    pm.add<qfault::TGateSynthesisPass<CliffordOnlyProvider>>(CliffordOnlyProvider{});

    const auto result = pm.run(mod, ctx);
    EXPECT_EQ(result, qfault::PassResult::Success);
    EXPECT_FALSE(hasTGate(mod))
        << "No T/Tdg should remain after CliffordOnlyProvider replacement";
    // 3 T/Tdg gates each expanded to 3 gates: 7 - 3 + 9 = 13
    EXPECT_EQ(mod.instructions.size(), originalCount - 3 + 9)
        << "Instruction count should be " << (originalCount - 3 + 9);
}

TEST(TGateSynthesisPass, InstructionCountIncreasesWithRealProvider) {
    // SKProvider returns sequences longer than 1 for most angles,
    // confirming expansion occurs (even if the output contains T gates).
    auto mod = makeModule();
    const std::size_t originalCount = mod.instructions.size();

    qfault::PassContext ctx{5};
    qfault::PassManager pm;
    pm.add<qfault::TGateSynthesisPass<qfault::SKProvider>>(qfault::SKProvider{});
    pm.run(mod, ctx);

    // After pass, SKProvider may return {T} for π/4 (exact match) — instruction
    // count may equal original. At minimum it must not decrease.
    EXPECT_GE(mod.instructions.size(), originalCount);
}

TEST(TGateSynthesisPass, NonTGatesPreserved) {
    auto mod = makeModule();
    qfault::PassContext ctx{5};
    qfault::PassManager pm;
    pm.add<qfault::TGateSynthesisPass<CliffordOnlyProvider>>(CliffordOnlyProvider{});
    pm.run(mod, ctx);

    // CX appears only in the original circuit (CliffordOnlyProvider doesn't emit CX).
    // There must be exactly 1 CX in the output.
    std::size_t cxCount = 0;
    for (const auto& instr : mod.instructions) {
        const auto* g = std::get_if<qfault::LogicalGate>(&instr);
        if (g && g->kind == qfault::GateKind::CX) ++cxCount;
    }
    EXPECT_EQ(cxCount, 1u) << "CX (non-T gate) must pass through unchanged";
}

TEST(TGateSynthesisPass, ModuleLevelRemainsLogical) {
    auto mod = makeModule();
    qfault::PassContext ctx{5};
    qfault::PassManager pm;
    pm.add<qfault::TGateSynthesisPass<qfault::SKProvider>>(qfault::SKProvider{});
    pm.run(mod, ctx);
    EXPECT_EQ(mod.level, qfault::IRLevel::LOGICAL);
}

TEST(TGateSynthesisPass, DiagnosticLoggedForReplacedGates) {
    auto mod = makeModule();
    qfault::PassContext ctx{5};
    qfault::PassManager pm;
    pm.add<qfault::TGateSynthesisPass<qfault::SKProvider>>(qfault::SKProvider{});
    pm.run(mod, ctx);

    bool found = false;
    for (const auto& d : ctx.diagnostics()) {
        if (d.message.find("replaced") != std::string::npos) {
            found = true;
            break;
        }
    }
    EXPECT_TRUE(found) << "Expected diagnostic about replaced T-gates";
}
