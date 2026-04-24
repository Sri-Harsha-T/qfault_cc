#pragma once

#include <qfault/ir/IRLevel.hpp>
#include <qfault/ir/QFaultIRModule.hpp>
#include <qfault/passes/PassBase.hpp>
#include <qfault/passes/PassContext.hpp>

#include <string_view>

namespace qfault {

class NoOpPass : public PassBase {
public:
    explicit NoOpPass(IRLevel level = IRLevel::LOGICAL) : level_(level) {}

    [[nodiscard]] std::string_view name() const override { return "NoOpPass"; }
    [[nodiscard]] IRLevel requiredLevel() const override { return level_; }

    PassResult run(QFaultIRModule& module, PassContext& /*ctx*/) override {
        module.assertLevel(requiredLevel());
        for (const auto& instr : module.instructions) {
            (void)instr; // reads without modifying
        }
        return PassResult::Success;
    }

private:
    IRLevel level_;
};

} // namespace qfault
