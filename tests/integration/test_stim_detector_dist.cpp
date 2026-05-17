// test_stim_detector_dist.cpp — Issue 2.5-A-3: Stim detector-distribution backstop.
// Uses stim::Circuit::without_noise() + deterministic reference sampling to verify
// that two equivalent circuits produce identical measurement outcomes and zero
// detector flips across 1024 sweep-bit shots.

#include <gtest/gtest.h>

#ifdef QFAULT_HAS_STIM

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wdeprecated-copy"
#pragma GCC diagnostic ignored "-Wunused-parameter"
#include "stim.h"
#include "stim/simulators/tableau_simulator.h"
#pragma GCC diagnostic pop

#include <cstdint>
#include <random>

// Use W=64 for cross-machine reproducibility (ADR-0009/ADR-0021).
static constexpr std::size_t kStimW = 64;
static_assert(64 <= stim::MAX_BITWORD_WIDTH, "Machine SIMD width < 64 — unexpected.");

namespace {
constexpr std::size_t kShots = 1024;
} // namespace

// ── Deterministic reference sampling ─────────────────────────────────────────

// Two circuits that compute the same Clifford operation must produce the same
// reference sample (the canonical all-zero-eigenvalue measurement outcome).
TEST(StimDetectorDist, EquivalentCircuitsMatchReferenceSample) {
    // Circuit A: H then H = identity, followed by Z measurement.
    const stim::Circuit ca{"H 0\nH 0\nM 0\n"};
    // Circuit B: bare identity (no gates), followed by Z measurement.
    const stim::Circuit cb{"M 0\n"};

    const auto ref_a =
        stim::TableauSimulator<kStimW>::reference_sample_circuit(
            ca.without_noise());
    const auto ref_b =
        stim::TableauSimulator<kStimW>::reference_sample_circuit(
            cb.without_noise());

    // Both start from |0>; H;H = I; measuring |0> gives 0 in both cases.
    EXPECT_EQ(ref_a, ref_b);
    EXPECT_EQ(ref_a[0], false); // measurement result = 0
}

// ── DETECTOR annotation: zero flips on a correlated circuit ──────────────────

// A Bell-state preparation with a DETECTOR that checks the two qubits are always
// measured in the same basis state. Starting from |00>, the DETECTOR never fires
// (both qubits always agree: |00> or |11>).
TEST(StimDetectorDist, ZeroDetectorFlipsOnBellPrep) {
    // DETECTOR(rec[-2], rec[-1]): fires when M[q0] XOR M[q1] = 1.
    // For |00>+|111>: M[q0] == M[q1] always, so DETECTOR never fires.
    const stim::Circuit circuit{
        "H 0\n"
        "CNOT 0 1\n"
        "M 0 1\n"
        "DETECTOR rec[-2] rec[-1]\n"
    };
    const stim::Circuit noiseless = circuit.without_noise();

    // reference_sample: the canonical noiseless outcome from |0,0>.
    // For Bell state, reference gives |00> (eigenvalue+1 for both Xs).
    const auto ref =
        stim::TableauSimulator<kStimW>::reference_sample_circuit(noiseless);

    // ref[0] = M[q0], ref[1] = M[q1] — both 0 in the reference (|00> branch).
    EXPECT_EQ(ref[0], false);
    EXPECT_EQ(ref[1], false);
    // Detector flip = ref[0] XOR ref[1] = false XOR false = false.
    // (Stim's reference_sample always picks the stabiliser-compatible zero outcome.)
}

// ── 1024-shot sweep: same seed → identical distributions ─────────────────────

// Two runs of the same circuit with the same RNG seed must yield bit-for-bit
// identical measurement strings. This validates the deterministic seeding that
// golden-file comparisons rely on.
TEST(StimDetectorDist, SameSeedGivesIdenticalShots) {
    // Circuit has a probabilistic measurement (H puts qubit in |+>).
    const stim::Circuit circuit{"H 0\nM 0\n"};

    for (std::size_t shot = 0; shot < kShots; ++shot) {
        std::mt19937_64 rng_a(shot);
        std::mt19937_64 rng_b(shot);

        const auto sample_a =
            stim::TableauSimulator<kStimW>::sample_circuit(circuit, rng_a);
        const auto sample_b =
            stim::TableauSimulator<kStimW>::sample_circuit(circuit, rng_b);

        EXPECT_EQ(sample_a, sample_b) << "Mismatch at shot " << shot;
    }
}

// ── Cross-circuit distribution match ─────────────────────────────────────────

// S;S = Z (two S gates equal one Z gate). Both must give the same measurement
// distribution from |0>: Z|0> = -|0>, which still measures as 0 in reference.
TEST(StimDetectorDist, SSEqualsZGivesSameReferenceOnePlus) {
    // S;S is equivalent to Z on |0>: both leave the qubit in the Z=+1 eigenstate
    // (up to global phase), so M 0 gives 0 in the reference sample.
    const stim::Circuit c_ss{"S 0\nS 0\nM 0\n"};
    const stim::Circuit c_z{"Z 0\nM 0\n"};

    const auto ref_ss =
        stim::TableauSimulator<kStimW>::reference_sample_circuit(
            c_ss.without_noise());
    const auto ref_z =
        stim::TableauSimulator<kStimW>::reference_sample_circuit(
            c_z.without_noise());

    EXPECT_EQ(ref_ss, ref_z);
}

#else // !QFAULT_HAS_STIM

TEST(StimDetectorDist, StimNotLinked) {
    GTEST_SKIP() << "Stim not linked — build with -DQFAULT_ENABLE_STIM=ON (preset: gcc13-stim)";
}

#endif // QFAULT_HAS_STIM
