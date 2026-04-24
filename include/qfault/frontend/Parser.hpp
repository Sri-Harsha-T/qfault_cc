#pragma once

#include <qfault/ir/QFaultIRModule.hpp>
#include <qfault/passes/PassContext.hpp>

#include <string_view>
#include <vector>

namespace qfault::frontend {

struct ParseResult {
    QFaultIRModule module;
    std::vector<Diagnostic> diagnostics;
};

class Parser {
public:
    // Parse a Clifford+T subset of OpenQASM 3.0 into a LOGICAL-level module.
    // Syntax errors recover by skipping to the next semicolon.
    // Unsupported gates produce DiagLevel::Warn diagnostics.
    [[nodiscard]] static ParseResult parse(std::string_view src);
};

} // namespace qfault::frontend
