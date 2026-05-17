// test_BV10_golden.cpp — Issue #53: BV-10 d=5 reference circuit golden files.
//
// Generates a noiseless d=5 rotated_memory_z surface code circuit via
// stim::generate_surface_code_circuit and validates committed goldens in
// bench/golden/bv10-d5/. Set QFAULT_UPDATE_GOLDENS=1 to regenerate.

#ifdef QFAULT_HAS_STIM

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wdeprecated-copy"
#pragma GCC diagnostic ignored "-Wunused-parameter"
#include "stim.h"
#include "stim/gen/gen_surface_code.h"
#include "stim/util_top/circuit_to_dem.h"
#pragma GCC diagnostic pop

#include <gtest/gtest.h>

#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <sstream>
#include <string>

namespace {

// d=5, 20 syndrome-extraction rounds, noiseless rotated memory (Z basis).
constexpr uint32_t kDist   = 5;
constexpr uint64_t kRounds = 20;

// Physical qubits (Pauli frame count) in a d=5 patch: 2d²-1 = 49.
// Stim's count_qubits() returns max_qubit_index+1 which can exceed this
// due to the 2D coordinate encoding used by generate_surface_code_circuit.
// We verify the range: ≥ 2d²-1 (minimum) and ≤ (2d-1)² (grid upper bound).
constexpr uint32_t kMinQubits = 2 * kDist * kDist - 1;          // 49
constexpr uint32_t kMaxQubits = (2 * kDist - 1) * (2 * kDist - 1); // 81

std::filesystem::path golden_dir() {
    // Locate bench/golden/bv10-d5/ relative to the repo root.
    // CMAKE sets QFAULT_REPO_ROOT; fall back to __FILE__ traversal.
    const char* root_env = std::getenv("QFAULT_REPO_ROOT");
    if (root_env) {
        return std::filesystem::path{root_env} / "bench" / "golden" / "bv10-d5";
    }
    // __FILE__ = tests/integration/test_BV10_golden.cpp
    std::filesystem::path here{__FILE__};
    return here.parent_path().parent_path().parent_path() / "bench" / "golden" / "bv10-d5";
}

std::string read_file(const std::filesystem::path& p) {
    std::ifstream f{p};
    if (!f) return {};
    std::ostringstream ss;
    ss << f.rdbuf();
    return ss.str();
}

void write_file(const std::filesystem::path& p, const std::string& content) {
    std::filesystem::create_directories(p.parent_path());
    std::ofstream f{p};
    f << content;
}

// Build the noiseless d=5 rotated_memory_z circuit.
stim::Circuit make_reference_circuit() {
    stim::CircuitGenParameters params{kRounds, kDist, "rotated_memory_z"};
    // All noise probabilities default to 0 in CircuitGenParameters.
    auto gen = stim::generate_surface_code_circuit(params);
    return gen.circuit;
}

// Reference sample for noiseless circuit: all detectors = 0, observable = 0.
// Sparse dets format: a shot with no detector fires is just "shot".
std::string make_reference_sample_dets() {
    return "shot\n";
}

} // namespace

// ── QFAULT_UPDATE_GOLDENS=1 ───────────────────────────────────────────────────

TEST(BV10Golden, GenerateOrVerifyGoldenFiles) {
    const bool update = (std::getenv("QFAULT_UPDATE_GOLDENS") != nullptr);

    const stim::Circuit circuit = make_reference_circuit();
    const auto dir = golden_dir();

    // ── circuit.stim ──────────────────────────────────────────────────────────
    std::ostringstream circuit_ss;
    circuit_ss << circuit;
    const std::string circuit_text = circuit_ss.str();

    if (update) {
        write_file(dir / "circuit.stim", circuit_text);
    } else {
        const std::string committed = read_file(dir / "circuit.stim");
        ASSERT_FALSE(committed.empty()) << "bench/golden/bv10-d5/circuit.stim not found — "
            "run with QFAULT_UPDATE_GOLDENS=1 to generate";
        EXPECT_EQ(committed, circuit_text) << "circuit.stim differs from regenerated circuit";
    }

    // ── circuit.dem ───────────────────────────────────────────────────────────
    const stim::DetectorErrorModel dem = stim::circuit_to_dem(circuit);
    std::ostringstream dem_ss;
    dem_ss << dem;
    const std::string dem_text = dem_ss.str();

    if (update) {
        write_file(dir / "circuit.dem", dem_text);
    } else {
        const std::string committed_dem = read_file(dir / "circuit.dem");
        ASSERT_FALSE(committed_dem.empty()) << "bench/golden/bv10-d5/circuit.dem not found";
        EXPECT_EQ(committed_dem, dem_text);
    }

    // ── reference_sample.dets ─────────────────────────────────────────────────
    const std::string dets_text = make_reference_sample_dets();
    if (update) {
        write_file(dir / "reference_sample.dets", dets_text);
    } else {
        const std::string committed_dets = read_file(dir / "reference_sample.dets");
        ASSERT_FALSE(committed_dets.empty()) << "bench/golden/bv10-d5/reference_sample.dets not found";
        EXPECT_EQ(committed_dets, dets_text);
    }

    // ── stim_version.txt ──────────────────────────────────────────────────────
    if (update) {
        write_file(dir / "stim_version.txt", "v1.15.0\n");
    } else {
        const std::string ver = read_file(dir / "stim_version.txt");
        EXPECT_EQ(ver, "v1.15.0\n") << "stim_version.txt mismatch";
    }
}

// ── Structural assertions (always run, even without goldens) ──────────────────

TEST(BV10Golden, ReferenceCircuitHasCorrectQubitCount) {
    const stim::Circuit circuit = make_reference_circuit();
    const uint32_t nq = static_cast<uint32_t>(circuit.count_qubits());
    // Stim uses coordinate-based qubit indices so count_qubits() ≥ 2d²-1.
    EXPECT_GE(nq, kMinQubits) << "circuit must use at least 2d²-1 = 49 qubit indices";
    EXPECT_LE(nq, kMaxQubits) << "circuit must not exceed (2d-1)² = 81 qubit indices";
}

TEST(BV10Golden, ReferenceCircuitHasDetectors) {
    const stim::Circuit circuit = make_reference_circuit();
    EXPECT_GT(circuit.count_detectors(), 0u)
        << "syndrome extraction circuit must have detectors";
}

TEST(BV10Golden, ReferenceCircuitHasOneObservable) {
    const stim::Circuit circuit = make_reference_circuit();
    // rotated_memory_z preserves one logical Z observable.
    EXPECT_EQ(circuit.count_observables(), 1u);
}

TEST(BV10Golden, ReferenceCircuitIsNoiseless) {
    // without_noise() must return the same circuit (no noise channels present).
    const stim::Circuit circuit = make_reference_circuit();
    const stim::Circuit clean   = circuit.without_noise();
    std::ostringstream ss_orig, ss_clean;
    ss_orig  << circuit;
    ss_clean << clean;
    EXPECT_EQ(ss_orig.str(), ss_clean.str())
        << "Reference circuit must be noiseless — noise was found";
}

TEST(BV10Golden, StimVersionFileContainsExpectedVersion) {
    const auto ver_path = golden_dir() / "stim_version.txt";
    const std::string content = read_file(ver_path);
    if (content.empty()) {
        GTEST_SKIP() << "stim_version.txt not committed yet — run with QFAULT_UPDATE_GOLDENS=1";
    }
    EXPECT_EQ(content, "v1.15.0\n");
}

#else // !QFAULT_HAS_STIM

#include <gtest/gtest.h>
TEST(BV10Golden, SkippedNoBuildFlag) {
    GTEST_SKIP() << "QFAULT_HAS_STIM not defined — BV10 golden tests skipped";
}

#endif // QFAULT_HAS_STIM
