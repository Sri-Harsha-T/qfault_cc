#ifdef QFAULT_HAS_STIM

#include <qfault/oracle/StimOracle.hpp>
#include <qfault/ir/GateKind.hpp>
#include <qfault/ir/LogicalGate.hpp>
#include <qfault/ir/IRLevel.hpp>

// stim.h and tableau header already included (with diagnostic suppression) via StimOracle.hpp.
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wdeprecated-copy"
#pragma GCC diagnostic ignored "-Wunused-parameter"
#include "stim/util_top/circuit_vs_tableau.h"
#include "stim/util_top/has_flow.h"
#pragma GCC diagnostic pop

#include <random>
#include <sstream>
#include <unordered_map>
#include <variant>

namespace qfault::oracle {

namespace {

// Map QFaultIR Clifford GateKind → Stim instruction name.
// Returns empty string for non-Clifford gates (T, Tdg, RZ, U).
std::string_view clifford_to_stim_name(GateKind k) noexcept {
    switch (k) {
        case GateKind::H:   return "H";
        case GateKind::X:   return "X";
        case GateKind::Y:   return "Y";
        case GateKind::Z:   return "Z";
        case GateKind::S:   return "S";
        case GateKind::Sdg: return "S_DAG";
        case GateKind::CX:  return "CNOT";
        case GateKind::CZ:  return "CZ";
        default:            return "";
    }
}

} // namespace

std::expected<std::string, std::string> ir_to_stim_text(const QFaultIRModule& mod) {
    // Build qubit-name → index map from the module's declared qubits.
    std::unordered_map<std::string, std::size_t> qubit_idx;
    qubit_idx.reserve(mod.qubits.size());
    for (std::size_t i = 0; i < mod.qubits.size(); ++i) {
        qubit_idx[mod.qubits[i].name] = i;
    }

    std::ostringstream out;
    for (const auto& instr : mod.instructions) {
        const auto* gate = std::get_if<LogicalGate>(&instr);
        if (!gate) {
            return std::unexpected("ir_to_stim_text: non-LogicalGate instruction");
        }

        std::string_view stim_name = clifford_to_stim_name(gate->kind);
        if (stim_name.empty()) {
            return std::unexpected(
                std::string("ir_to_stim_text: non-Clifford gate (kind=") +
                std::to_string(static_cast<int>(gate->kind)) + ")");
        }

        out << stim_name;
        for (const auto& q : gate->operands) {
            auto it = qubit_idx.find(q.name);
            if (it == qubit_idx.end()) {
                return std::unexpected("ir_to_stim_text: unknown qubit '" + q.name + "'");
            }
            out << ' ' << it->second;
        }
        out << '\n';
    }
    return out.str();
}

std::expected<bool, std::string> circuits_clifford_equivalent(
    const QFaultIRModule& a, const QFaultIRModule& b) {

    if (a.qubits.size() != b.qubits.size()) {
        return std::unexpected("circuits_clifford_equivalent: qubit count mismatch (" +
            std::to_string(a.qubits.size()) + " vs " +
            std::to_string(b.qubits.size()) + ")");
    }

    auto text_a = ir_to_stim_text(a);
    if (!text_a) return std::unexpected(text_a.error());

    auto text_b = ir_to_stim_text(b);
    if (!text_b) return std::unexpected(text_b.error());

    // Stim infers qubit count from instruction operands. An empty circuit has
    // num_qubits=0, so its tableau cannot equal a non-trivial circuit's tableau
    // even when both represent the identity. Anchor any empty circuit to the
    // declared qubit count by prepending I on all qubits (identity — no effect).
    auto anchor_qubits = [](std::string& text, std::size_t n) {
        if (text.empty() && n > 0) {
            text = "I";
            for (std::size_t i = 0; i < n; ++i) { text += ' '; text += std::to_string(i); }
            text += '\n';
        }
    };
    anchor_qubits(*text_a, a.qubits.size());
    anchor_qubits(*text_b, b.qubits.size());

    const stim::Circuit ca{std::string_view{*text_a}};
    const stim::Circuit cb{std::string_view{*text_b}};

    // circuit_to_tableau<W> compiles the circuit into a Pauli-frame tableau.
    // Tableau::operator== is a deterministic, exact comparison — no sampling.
    // W=MAX_BITWORD_WIDTH=64 (enforced by static_assert in StimOracle.hpp).
    const auto ta = stim::circuit_to_tableau<kStimW>(
        ca, /*ignore_noise=*/false, /*ignore_meas=*/false, /*ignore_reset=*/false);
    const auto tb = stim::circuit_to_tableau<kStimW>(
        cb, /*ignore_noise=*/false, /*ignore_meas=*/false, /*ignore_reset=*/false);

    return ta == tb;
}

// ── has_flow oracle (ADR-0021) ────────────────────────────────────────────────

std::expected<bool, std::string>
checkHasFlow(const stim::Circuit& circuit,
             std::span<const stim::Flow<kStimW>> flows,
             std::size_t num_samples) {
    if (flows.empty()) return true;

    // Fixed seed for cross-machine reproducibility (ADR-0021).
    std::mt19937_64 rng{42};

    try {
        const auto results = stim::sample_if_circuit_has_stabilizer_flows<kStimW>(
            num_samples, rng, circuit, flows);
        for (const bool ok : results) {
            if (!ok) return false;
        }
        return true;
    } catch (const std::exception& e) {
        return std::unexpected(std::string("checkHasFlow: ") + e.what());
    }
}

std::expected<bool, std::string>
checkDetectorMatch(const stim::Circuit& circuit_a,
                   const stim::Circuit& circuit_b,
                   std::size_t /*num_shots*/) {
    const stim::Circuit clean_a = circuit_a.without_noise();
    const stim::Circuit clean_b = circuit_b.without_noise();

    // Sweep-bit sampling for measured circuits is deferred (#52-ext).
    if (clean_a.count_measurements() > 0 || clean_b.count_measurements() > 0) {
        return std::unexpected(
            "checkDetectorMatch: sweep-bit sampling for measured circuits "
            "is deferred to #52-ext");
    }

    try {
        const auto ta = stim::circuit_to_tableau<kStimW>(
            clean_a, /*ignore_noise=*/false,
            /*ignore_meas=*/false, /*ignore_reset=*/false);
        const auto tb = stim::circuit_to_tableau<kStimW>(
            clean_b, /*ignore_noise=*/false,
            /*ignore_meas=*/false, /*ignore_reset=*/false);
        return ta == tb;
    } catch (const std::exception& e) {
        return std::unexpected(std::string("checkDetectorMatch: ") + e.what());
    }
}

} // namespace qfault::oracle

#endif // QFAULT_HAS_STIM
