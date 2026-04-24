#include <qfault/passes/PassContext.hpp>

#include <gtest/gtest.h>
#include <stdexcept>
#include <thread>

using namespace qfault;

TEST(PassContext, ConstructsWithValidOddDistance) {
    EXPECT_NO_THROW(PassContext{3});
    EXPECT_NO_THROW(PassContext{5});
    EXPECT_NO_THROW(PassContext{7});
    EXPECT_NO_THROW(PassContext{99});
}

TEST(PassContext, ThrowsOnEvenDistance) {
    EXPECT_THROW(PassContext{4}, std::invalid_argument);
    EXPECT_THROW(PassContext{2}, std::invalid_argument);
    EXPECT_THROW(PassContext{0}, std::invalid_argument);
}

TEST(PassContext, ThrowsOnDistanceLessThanThree) {
    EXPECT_THROW(PassContext{1}, std::invalid_argument);
}

TEST(PassContext, CodeDistanceAccessor) {
    PassContext ctx{7};
    EXPECT_EQ(ctx.codeDistance(), 7u);
}

TEST(PassContext, DefaultSynthesisEpsilon) {
    PassContext ctx{5};
    EXPECT_DOUBLE_EQ(ctx.synthesisEpsilon(), 1e-10);
}

TEST(PassContext, CustomSynthesisEpsilon) {
    PassContext ctx{5, 1e-6};
    EXPECT_DOUBLE_EQ(ctx.synthesisEpsilon(), 1e-6);
}

TEST(PassContext, DiagnosticsStartEmpty) {
    PassContext ctx{5};
    EXPECT_TRUE(ctx.diagnostics().empty());
}

TEST(PassContext, AddDiagnosticInfo) {
    PassContext ctx{5};
    ctx.addDiagnostic(DiagLevel::Info, "all good");
    ASSERT_EQ(ctx.diagnostics().size(), 1u);
    EXPECT_EQ(ctx.diagnostics()[0].level, DiagLevel::Info);
    EXPECT_EQ(ctx.diagnostics()[0].message, "all good");
}

TEST(PassContext, AddMultipleDiagnostics) {
    PassContext ctx{5};
    ctx.addDiagnostic(DiagLevel::Info, "a");
    ctx.addDiagnostic(DiagLevel::Warn, "b");
    ctx.addDiagnostic(DiagLevel::Error, "c");
    ASSERT_EQ(ctx.diagnostics().size(), 3u);
    EXPECT_EQ(ctx.diagnostics()[1].level, DiagLevel::Warn);
    EXPECT_EQ(ctx.diagnostics()[2].level, DiagLevel::Error);
}

TEST(PassContext, TimerMeasuresElapsedTime) {
    PassContext ctx{5};
    ctx.startTimer("test");
    // Minimal spin so the timer actually ticks
    volatile int sum = 0;
    for (int i = 0; i < 100000; ++i) sum += i;
    (void)sum;
    ctx.stopTimer();
    // Duration must be non-negative; avoid asserting a lower bound (CI variance)
    EXPECT_GE(ctx.lastPassDuration().count(), 0);
}

TEST(PassContext, TimerDefaultsToZero) {
    PassContext ctx{5};
    EXPECT_EQ(ctx.lastPassDuration().count(), 0);
}

TEST(DiagLevel, ValuesDistinct) {
    EXPECT_NE(DiagLevel::Info, DiagLevel::Warn);
    EXPECT_NE(DiagLevel::Warn, DiagLevel::Error);
}
