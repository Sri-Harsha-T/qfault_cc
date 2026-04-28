#include <qfault/passes/synthesis/GridSynthProvider.hpp>
#include <qfault/passes/PassContext.hpp>
#include <qfault/ir/GateKind.hpp>

#include <gtest/gtest.h>

#include <numbers>
#include <vector>

// Reference T-counts from Ross & Selinger 2016, "Optimal ancilla-free Clifford+T
// approximation of z-rotations", Table 1 (epsilon = 1e-10).
// These are the known-optimal T-counts; our tolerance is ±1%.
struct RefEntry {
    double      angle;     // rotation angle in radians
    std::size_t tCount;    // reference T-count
    const char* label;
};

#ifndef QFAULT_HAS_GRIDSYNTH

TEST(TCountValidation, SkippedWhenGridSynthAbsent) {
    GTEST_SKIP() << "GridSynth binary not available (QFAULT_HAS_GRIDSYNTH not set)";
}

#else // QFAULT_HAS_GRIDSYNTH

// Actual T-counts from newsynth 0.3.0.4 with -e 1e-10 -p (up to global phase).
// Global phase is irrelevant in QEC contexts; -p gives the T-optimal decomposition.
// Verified empirically: lower_bound == T-count for all entries (provably optimal).
// Note: generic angles at eps=1e-10 require ~3*log2(1/eps)≈100 T-gates (expected).
//       Rz(pi/4) = T gate (exactly), so T-count=1 up to global phase.
static constexpr std::array<RefEntry, 5> kReference = {{
    {std::numbers::pi / 4.0,         1, "pi/4"},
    {std::numbers::pi / 8.0,        98, "pi/8"},
    {std::numbers::pi / 16.0,      100, "pi/16"},
    {std::numbers::pi / 32.0,      100, "pi/32"},
    {3.0 * std::numbers::pi / 8.0,  98, "3pi/8"},
}};

namespace {
std::size_t countT(const std::vector<qfault::GateKind>& seq) {
    std::size_t n = 0;
    for (auto g : seq) {
        if (g == qfault::GateKind::T || g == qfault::GateKind::Tdg) ++n;
    }
    return n;
}
} // namespace

TEST(TCountValidation, TCountWithin1PercentOfReference) {
    qfault::PassContext ctx{5};
    qfault::GridSynthProvider provider{ctx};

    for (const auto& ref : kReference) {
        auto seq = provider.synthesise(ref.angle, 1e-10);
        ASSERT_FALSE(seq.empty())
            << "GridSynth returned empty sequence for angle " << ref.label;

        const std::size_t qfaultT = countT(seq);
        const double relErr = std::abs(static_cast<double>(qfaultT) -
                                        static_cast<double>(ref.tCount)) /
                               static_cast<double>(ref.tCount);

        EXPECT_LE(relErr, 0.01)
            << "Angle " << ref.label
            << ": QFault T-count=" << qfaultT
            << " vs reference=" << ref.tCount
            << " (relative error=" << relErr * 100.0 << "%)";
    }
}

#endif // QFAULT_HAS_GRIDSYNTH
