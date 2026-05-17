#pragma once

#include <qfault/ir/IRLevel.hpp>
#include <qfault/ir/LogicalGate.hpp>
#include <qfault/ir/LogicalQubit.hpp>
#include <qfault/ir/PatchOp.hpp>
#include <qfault/ir/PauliFrameUpdate.hpp>

#include <stdexcept>
#include <string>
#include <variant>
#include <vector>

namespace qfault {

// Instruction = logical gate | physical patch op | classical Pauli-frame annotation.
// PauliFrameUpdate only appears in PHYSICAL-level modules (after LatticeSurgeryPass).
using Instruction = std::variant<LogicalGate, PatchOp, PauliFrameUpdate>;

struct QFaultIRModule {
    std::string name = {};
    IRLevel level{IRLevel::LOGICAL};
    std::vector<LogicalQubit> qubits = {};
    std::vector<Instruction> instructions = {};

    // Throws std::logic_error if the module is not at the expected level.
    // Call at the start of every PassBase::run() implementation.
    void assertLevel(IRLevel required) const {
        if (level != required) {
            throw std::logic_error(
                level == IRLevel::LOGICAL
                    ? "Pass requires PHYSICAL-level module but got LOGICAL"
                    : "Pass requires LOGICAL-level module but got PHYSICAL");
        }
    }

    [[nodiscard]] std::string dumpToString() const;
    void dump(std::ostream& out) const;
};

} // namespace qfault
