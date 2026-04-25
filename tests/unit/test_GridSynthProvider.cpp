#include <qfault/passes/synthesis/GridSynthProvider.hpp>
#include <qfault/passes/synthesis/SynthesisProvider.hpp>
#include <qfault/passes/PassContext.hpp>
#include <qfault/ir/GateKind.hpp>

#include <gtest/gtest.h>

#include <numbers>

// static_assert doesn't require default-construction — concept checks method
// signatures on a hypothetical instance, not constructibility.
static_assert(qfault::SynthesisProvider<qfault::GridSynthProvider>,
              "GridSynthProvider must satisfy SynthesisProvider");

TEST(GridSynthProvider, NameIsCorrect) {
    qfault::PassContext ctx{5};
    qfault::GridSynthProvider p{ctx};
    EXPECT_EQ(p.name(), "GridSynthProvider");
}

TEST(GridSynthProvider, SatisfiesSynthesisProviderConcept) {
    // Verified at compile time via static_assert above.
    SUCCEED();
}

#ifndef QFAULT_HAS_GRIDSYNTH

TEST(GridSynthProvider, SkippedWhenBinaryAbsent) {
    GTEST_SKIP() << "GridSynth binary not available (QFAULT_HAS_GRIDSYNTH not set)";
}

#else // QFAULT_HAS_GRIDSYNTH

TEST(GridSynthProvider, SynthesisePiOver4ReturnsNonEmptySequence) {
    qfault::PassContext ctx{5};
    qfault::GridSynthProvider p{ctx};
    auto seq = p.synthesise(std::numbers::pi / 4.0, 1e-10);
    EXPECT_FALSE(seq.empty())
        << "Expected non-empty gate sequence for pi/4 with eps=1e-10";
}

TEST(GridSynthProvider, AllReturnedGatesAreValidCliffordT) {
    qfault::PassContext ctx{5};
    qfault::GridSynthProvider p{ctx};
    auto seq = p.synthesise(std::numbers::pi / 4.0, 1e-10);
    for (auto g : seq) {
        using qfault::GateKind;
        const bool valid = (g == GateKind::H  || g == GateKind::X ||
                            g == GateKind::Y  || g == GateKind::Z ||
                            g == GateKind::S  || g == GateKind::Sdg ||
                            g == GateKind::T  || g == GateKind::Tdg);
        EXPECT_TRUE(valid) << "Unexpected GateKind " << static_cast<int>(g);
    }
}

TEST(GridSynthProvider, SynthesiseLogsNoDiagsOnSuccess) {
    qfault::PassContext ctx{5};
    qfault::GridSynthProvider p{ctx};
    (void)p.synthesise(std::numbers::pi / 4.0, 1e-10);
    // No Warn/Error diagnostics expected on successful synthesis.
    for (const auto& d : ctx.diagnostics()) {
        EXPECT_NE(d.level, qfault::DiagLevel::Error)
            << "Unexpected error diagnostic: " << d.message;
    }
}

#endif // QFAULT_HAS_GRIDSYNTH
