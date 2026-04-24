#pragma once

#include <qfault/ir/QFaultIRModule.hpp>
#include <qfault/passes/PassBase.hpp>
#include <qfault/passes/PassContext.hpp>

#include <chrono>
#include <memory>
#include <ostream>
#include <string>
#include <type_traits>
#include <vector>

namespace qfault {

struct PassStat {
    std::string name;
    std::chrono::microseconds duration{0};
    std::size_t instructionCount{0};
};

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
        stats_.clear();
        for (auto& pass : passes_) {
            ctx.startTimer(pass->name());
            PassResult result = pass->run(module, ctx);
            ctx.stopTimer();
            stats_.push_back(PassStat{
                std::string{pass->name()},
                ctx.lastPassDuration(),
                module.instructions.size(),
            });
            if (result == PassResult::Failure) {
                return PassResult::Failure;
            }
        }
        return PassResult::Success;
    }

    void printStats(std::ostream& out) const {
        out << "Pass               | Duration (µs) | Instructions\n"
            << "-------------------|---------------|-------------\n";
        for (const auto& s : stats_) {
            // Fixed 19/15/13 column widths — adequate for v0.1 diagnostics
            out << s.name;
            for (std::size_t i = s.name.size(); i < 19; ++i) out << ' ';
            out << "| ";
            std::string dur = std::to_string(s.duration.count());
            for (std::size_t i = dur.size(); i < 13; ++i) out << ' ';
            out << dur << " | ";
            std::string cnt = std::to_string(s.instructionCount);
            for (std::size_t i = cnt.size(); i < 12; ++i) out << ' ';
            out << cnt << '\n';
        }
    }

    [[nodiscard]] const std::vector<PassStat>& stats() const noexcept { return stats_; }

private:
    std::vector<std::unique_ptr<PassBase>> passes_;
    std::vector<PassStat> stats_;
};

} // namespace qfault
