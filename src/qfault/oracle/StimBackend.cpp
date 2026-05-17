#ifdef QFAULT_HAS_STIM

#include <qfault/oracle/StimBackend.hpp>
#include <qfault/oracle/StimOracle.hpp>

namespace qfault::oracle {

std::expected<stim::Circuit, std::string>
emit_logical_stim(const QFaultIRModule& mod) {
    if (mod.level != IRLevel::LOGICAL) {
        return std::unexpected(
            std::string("emit_logical_stim: expected LOGICAL IR, got ") +
            (mod.level == IRLevel::PHYSICAL ? "PHYSICAL" : "unknown"));
    }

    auto text_result = ir_to_stim_text(mod);
    if (!text_result) return std::unexpected(text_result.error());

    std::string& text = *text_result;

    // Anchor empty circuits: stim::Circuit infers num_qubits from instruction
    // operands. An all-idle module with no gates has an empty text, giving a
    // 0-qubit circuit whose flows cannot be compared to declared qubits.
    if (text.empty() && !mod.qubits.empty()) {
        text = "I";
        for (std::size_t i = 0; i < mod.qubits.size(); ++i) {
            text += ' '; text += std::to_string(i);
        }
        text += '\n';
    }

    return stim::Circuit{std::string_view{text}};
}

stim::Flow<kStimW>
make_z_identity_flow(std::size_t qubit_index, std::size_t /*num_qubits*/) {
    // "Z<k> -> Z<k>" — identity flow for logical Z observable of qubit k.
    const std::string s = "Z" + std::to_string(qubit_index) +
                          " -> Z" + std::to_string(qubit_index);
    return stim::Flow<kStimW>::from_str(s);
}

} // namespace qfault::oracle

#endif // QFAULT_HAS_STIM
