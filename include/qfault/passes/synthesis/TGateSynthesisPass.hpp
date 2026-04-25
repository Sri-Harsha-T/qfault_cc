#pragma once

#include <qfault/ir/GateKind.hpp>
#include <qfault/ir/IRLevel.hpp>
#include <qfault/ir/LogicalGate.hpp>
#include <qfault/ir/QFaultIRModule.hpp>
#include <qfault/passes/PassBase.hpp>
#include <qfault/passes/PassContext.hpp>
#include <qfault/passes/synthesis/SynthesisProvider.hpp>
#include <qfault/util/Overload.hpp>

#include <numbers>
#include <string>
#include <string_view>
#include <vector>

namespace qfault {

// Replaces every T/Tdg gate in a LOGICAL-level module with a Clifford+T
// sequence produced by ProviderT. Non-T/Tdg instructions are preserved unchanged.
//
// Inherits PassBase so it can be stored in PassManager alongside other passes.
// The virtual dispatch is at the pass granularity (once per run()), not per gate.
template <SynthesisProvider ProviderT>
class TGateSynthesisPass : public PassBase {
public:
    explicit TGateSynthesisPass(ProviderT provider)
        : provider_{std::move(provider)},
          name_{"TGateSynthesisPass<" + std::string{provider_.name()} + ">"} {}

    [[nodiscard]] std::string_view name() const override { return name_; }
    [[nodiscard]] IRLevel requiredLevel() const override { return IRLevel::LOGICAL; }

    PassResult run(QFaultIRModule& module, PassContext& ctx) override {
        module.assertLevel(IRLevel::LOGICAL);

        std::size_t replacedCount = 0;
        std::vector<Instruction> rebuilt;
        rebuilt.reserve(module.instructions.size());

        for (const auto& instr : module.instructions) {
            const auto* gate = std::get_if<LogicalGate>(&instr);
            if (gate && (gate->kind == GateKind::T || gate->kind == GateKind::Tdg)) {
                const double angle = (gate->kind == GateKind::T)
                                         ? (std::numbers::pi / 4.0)
                                         : (-std::numbers::pi / 4.0);
                auto seq = provider_.synthesise(angle, ctx.synthesisEpsilon());
                for (GateKind gk : seq) {
                    rebuilt.push_back(LogicalGate{.kind    = gk,
                                                  .operands = gate->operands});
                }
                ++replacedCount;
            } else {
                rebuilt.push_back(instr);
            }
        }

        module.instructions = std::move(rebuilt);

        if (replacedCount > 0) {
            ctx.addDiagnostic(DiagLevel::Info,
                "TGateSynthesisPass replaced " + std::to_string(replacedCount) +
                " T/Tdg gate(s)");
        }

        return PassResult::Success;
    }

private:
    ProviderT   provider_;
    std::string name_;
};

} // namespace qfault
