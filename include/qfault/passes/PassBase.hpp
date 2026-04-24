#pragma once

#include <qfault/ir/IRLevel.hpp>
#include <qfault/ir/QFaultIRModule.hpp>

#include <string_view>

namespace qfault {

class PassContext; // forward declaration — avoids circular include

enum class PassResult { Success, Failure };

class PassBase {
public:
    virtual ~PassBase() = default;

    [[nodiscard]] virtual std::string_view name() const = 0;
    [[nodiscard]] virtual IRLevel requiredLevel() const = 0;
    virtual PassResult run(QFaultIRModule& module, PassContext& ctx) = 0;
};

} // namespace qfault
