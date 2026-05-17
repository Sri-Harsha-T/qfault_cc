#ifdef QFAULT_HAS_QCEC

#include <qfault/oracle/QCECBridge.hpp>

// Suppress QCEC/mqt-core internal warnings that fire under -Wall -Wextra.
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wdeprecated-copy"
#pragma GCC diagnostic ignored "-Wunused-parameter"
#pragma GCC diagnostic ignored "-Wshadow"
#pragma GCC diagnostic ignored "-Wold-style-cast"
#include "Configuration.hpp"
#include "EquivalenceCheckingManager.hpp"
#include "EquivalenceCriterion.hpp"
#include "qasm3/Importer.hpp"
#pragma GCC diagnostic pop

#include <sstream>

namespace qfault::oracle {

std::string_view to_string(EquivalenceResult r) noexcept {
    switch (r) {
        case EquivalenceResult::NotEquivalent:            return "not_equivalent";
        case EquivalenceResult::Equivalent:               return "equivalent";
        case EquivalenceResult::NoInformation:            return "no_information";
        case EquivalenceResult::ProbablyEquivalent:       return "probably_equivalent";
        case EquivalenceResult::EquivalentUpToGlobalPhase: return "equivalent_up_to_global_phase";
        case EquivalenceResult::EquivalentUpToPhase:      return "equivalent_up_to_phase";
        case EquivalenceResult::ProbablyNotEquivalent:    return "probably_not_equivalent";
        default:                                          return "no_information";
    }
}

static EquivalenceResult convert(ec::EquivalenceCriterion c) noexcept {
    return static_cast<EquivalenceResult>(static_cast<unsigned char>(c));
}

EquivalenceResult check_equivalence(const std::string& qasmA,
                                    const std::string& qasmB,
                                    std::size_t /*nqubits*/) {
    // Parse QASM 3.0 strings using mqt-core's importer.
    const auto qc1 = qasm3::Importer::imports(qasmA);
    const auto qc2 = qasm3::Importer::imports(qasmB);

    // Configuration: Construction checker OFF; Alternating + Simulation + ZX ON.
    ec::Configuration config{};
    config.execution.runConstructionChecker = false;
    config.execution.runAlternatingChecker  = true;
    config.execution.runSimulationChecker   = true;
    config.execution.runZXChecker           = true;
    config.execution.parallel               = false;

    ec::EquivalenceCheckingManager ecm{qc1, qc2, config};
    ecm.run();

    return convert(ecm.getResults().equivalence);
}

} // namespace qfault::oracle

#endif // QFAULT_HAS_QCEC
