#include <qfault/ir/QFaultIRModule.hpp>
#include <qfault/util/Overload.hpp>

#include <ostream>
#include <sstream>
#include <string_view>

namespace qfault {

namespace {

std::string_view gateKindName(GateKind k) {
    switch (k) {
    case GateKind::H:   return "H";
    case GateKind::X:   return "X";
    case GateKind::Y:   return "Y";
    case GateKind::Z:   return "Z";
    case GateKind::S:   return "S";
    case GateKind::Sdg: return "Sdg";
    case GateKind::T:   return "T";
    case GateKind::Tdg: return "Tdg";
    case GateKind::CX:  return "CX";
    case GateKind::CZ:  return "CZ";
    case GateKind::RZ:  return "RZ";
    case GateKind::U:   return "U";
    }
    return "?";
}

std::string_view patchOpKindName(PatchOpKind k) {
    switch (k) {
    case PatchOpKind::MERGE:   return "MERGE";
    case PatchOpKind::SPLIT:   return "SPLIT";
    case PatchOpKind::MEASURE: return "MEASURE";
    case PatchOpKind::IDLE:    return "IDLE";
    }
    return "?";
}

std::string_view measBasisName(MeasBasis b) {
    switch (b) {
    case MeasBasis::X: return "X";
    case MeasBasis::Z: return "Z";
    }
    return "?";
}

} // namespace

void QFaultIRModule::dump(std::ostream& out) const {
    out << "; Module: " << name
        << " [" << (level == IRLevel::LOGICAL ? "LOGICAL" : "PHYSICAL") << "]\n";

    if (level == IRLevel::LOGICAL) {
        out << "; Qubits:";
        for (const auto& q : qubits) {
            out << " " << q.name << "[" << q.index << "]";
        }
        out << "\n";
    }

    for (const auto& instr : instructions) {
        std::visit(overload{
            [&out](const LogicalGate& g) {
                out << gateKindName(g.kind);
                for (const auto& q : g.operands) {
                    out << " " << q.name << "[" << q.index << "]";
                }
                if (g.angle.has_value()) {
                    out << " (" << *g.angle << ")";
                }
                out << "\n";
            },
            [&out](const PatchOp& op) {
                out << patchOpKindName(op.kind);
                for (const auto& p : op.patches) {
                    out << " (" << p.x << "," << p.y << ")";
                }
                out << " basis=" << measBasisName(op.basis)
                    << " @t=" << op.timeStep << "\n";
            },
        }, instr);
    }
}

std::string QFaultIRModule::dumpToString() const {
    std::ostringstream oss;
    dump(oss);
    return oss.str();
}

} // namespace qfault
