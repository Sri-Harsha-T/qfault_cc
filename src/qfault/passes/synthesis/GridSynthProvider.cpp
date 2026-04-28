#include <qfault/passes/synthesis/GridSynthProvider.hpp>

#include <array>
#include <cstdio>
#include <iomanip>
#include <limits>
#include <optional>
#include <sstream>
#include <string>

namespace qfault {

namespace {

#ifdef GRIDSYNTH_BINARY
// Parse GridSynth stdout into a GateKind sequence.
// GridSynth outputs a string of gate characters, e.g. "HTTHTSTH".
// Characters:
//   H=Hadamard, T=T, S=S, X=Pauli-X, Y=Pauli-Y, Z=Pauli-Z, W=omega (ignore)
//   Lowercase variants are inverse: t=Tdg, s=Sdg
// Returns nullopt if the output is empty or contains only whitespace.
std::optional<std::vector<GateKind>> parseGridSynthOutput(const std::string& out) {
    std::vector<GateKind> seq;
    for (char c : out) {
        switch (c) {
            case 'H': seq.push_back(GateKind::H);   break;
            case 'T': seq.push_back(GateKind::T);   break;
            case 't': seq.push_back(GateKind::Tdg); break;
            case 'S': seq.push_back(GateKind::S);   break;
            case 's': seq.push_back(GateKind::Sdg); break;
            case 'X': seq.push_back(GateKind::X);   break;
            case 'Y': seq.push_back(GateKind::Y);   break;
            case 'Z': seq.push_back(GateKind::Z);   break;
            case 'W': break; // global phase — irrelevant for gate sequences
            case '\n': case '\r': case ' ': case '\t': break; // whitespace
            default:  break; // unknown character — skip
        }
    }
    if (seq.empty()) return std::nullopt;
    return seq;
}

// Run the gridsynth binary with the given angle and epsilon.
// Returns the stdout as a string, or nullopt on failure.
std::optional<std::string> runGridSynth(const char* binary,
                                        double angle,
                                        double eps) {
    // -p: decompose up to global phase (global phase is irrelevant in QEC contexts)
    // Full precision is required: default ostringstream truncates to 6 sig figs,
    // giving gridsynth a slightly wrong angle and non-optimal T-counts.
    std::ostringstream cmd;
    cmd << std::setprecision(std::numeric_limits<double>::max_digits10);
    cmd << "\"" << binary << "\" -e " << eps << " -p " << angle << " 2>/dev/null";
    const std::string cmdStr = cmd.str();

    // NOLINTNEXTLINE(cert-env33-c) — controlled binary path from build system
    std::FILE* pipe = ::popen(cmdStr.c_str(), "r");
    if (!pipe) return std::nullopt;

    std::string result;
    std::array<char, 256> buf{};
    while (std::fgets(buf.data(), static_cast<int>(buf.size()), pipe)) {
        result += buf.data();
    }
    const int status = ::pclose(pipe);
    if (status != 0) return std::nullopt;
    return result;
}
#endif // GRIDSYNTH_BINARY

} // namespace

std::vector<GateKind> GridSynthProvider::synthesise(double angle, double eps) { // NOLINT
#ifndef GRIDSYNTH_BINARY
    (void)angle;
    (void)eps;
    ctx_.addDiagnostic(DiagLevel::Warn,
        "GridSynthProvider: GRIDSYNTH_BINARY not set at build time; "
        "returning empty sequence");
    return {};
#else
    constexpr const char* kBinary = GRIDSYNTH_BINARY;
    if (kBinary[0] == '\0') {
        ctx_.addDiagnostic(DiagLevel::Warn,
            "GridSynthProvider: GRIDSYNTH_BINARY is empty; "
            "returning empty sequence");
        return {};
    }

    auto rawOut = runGridSynth(kBinary, angle, eps);
    if (!rawOut) {
        ctx_.addDiagnostic(DiagLevel::Warn,
            "GridSynthProvider: gridsynth binary failed or not found; "
            "returning empty sequence");
        return {};
    }

    auto parsed = parseGridSynthOutput(*rawOut);
    if (!parsed) {
        ctx_.addDiagnostic(DiagLevel::Warn,
            "GridSynthProvider: could not parse gridsynth output; "
            "returning empty sequence");
        return {};
    }

    return *parsed;
#endif
}

} // namespace qfault
