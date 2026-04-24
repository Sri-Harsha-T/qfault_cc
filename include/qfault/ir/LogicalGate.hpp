#pragma once

#include <qfault/ir/GateKind.hpp>
#include <qfault/ir/LogicalQubit.hpp>

#include <optional>
#include <vector>

namespace qfault {

struct LogicalGate {
    GateKind kind{GateKind::H};
    std::vector<LogicalQubit> operands;
    // Set only for rotation gates (RZ, U). Nullopt for Clifford and T gates.
    std::optional<double> angle = std::nullopt;

    [[nodiscard]] bool operator==(const LogicalGate&) const noexcept = default;
};

} // namespace qfault
