#pragma once

#include <qfault/ir/QFaultIRModule.hpp>
#include <qfault/passes/PassBase.hpp>
#include <qfault/passes/PassContext.hpp>

#include <memory>
#include <type_traits>
#include <vector>

namespace qfault {

class PassManager {
public:
    template <typename PassT, typename... Args>
    PassManager& add(Args&&... args) {
        static_assert(std::is_base_of_v<PassBase, PassT>,
                      "PassT must derive from PassBase");
        passes_.push_back(std::make_unique<PassT>(std::forward<Args>(args)...));
        return *this;
    }

    PassResult run(QFaultIRModule& module, PassContext& ctx) {
        for (auto& pass : passes_) {
            ctx.startTimer(pass->name());
            PassResult result = pass->run(module, ctx);
            ctx.stopTimer();
            if (result == PassResult::Failure) {
                return PassResult::Failure;
            }
        }
        return PassResult::Success;
    }

private:
    std::vector<std::unique_ptr<PassBase>> passes_;
};

} // namespace qfault
