#include <qfault/ir/IRLevel.hpp>
#include <qfault/ir/QFaultIRModule.hpp>
#include <qfault/passes/PassBase.hpp>
#include <qfault/passes/PassContext.hpp>
#include <qfault/passes/PassManager.hpp>

#include <gtest/gtest.h>
#include <sstream>
#include <string_view>
#include <vector>

using namespace qfault;

// ── Stub passes ───────────────────────────────────────────────────────────────

class CountingPass : public PassBase {
public:
    explicit CountingPass(std::vector<int>& log, int id)
        : log_(log), id_(id) {}

    [[nodiscard]] std::string_view name() const override { return "CountingPass"; }
    [[nodiscard]] IRLevel requiredLevel() const override { return IRLevel::LOGICAL; }

    PassResult run(QFaultIRModule& module, PassContext& /*ctx*/) override {
        module.assertLevel(requiredLevel());
        log_.push_back(id_);
        return PassResult::Success;
    }

private:
    std::vector<int>& log_;
    int id_;
};

class FailingPass : public PassBase {
public:
    [[nodiscard]] std::string_view name() const override { return "FailingPass"; }
    [[nodiscard]] IRLevel requiredLevel() const override { return IRLevel::LOGICAL; }
    PassResult run(QFaultIRModule& /*module*/, PassContext& /*ctx*/) override {
        return PassResult::Failure;
    }
};

// ── Tests ─────────────────────────────────────────────────────────────────────

TEST(PassManager, EmptyManagerReturnsSuccess) {
    PassManager pm;
    QFaultIRModule m{.level = IRLevel::LOGICAL};
    PassContext ctx{5};
    EXPECT_EQ(pm.run(m, ctx), PassResult::Success);
}

TEST(PassManager, SinglePassRunsAndSucceeds) {
    std::vector<int> log;
    PassManager pm;
    pm.add<CountingPass>(log, 1);

    QFaultIRModule m{.level = IRLevel::LOGICAL};
    PassContext ctx{5};
    EXPECT_EQ(pm.run(m, ctx), PassResult::Success);
    ASSERT_EQ(log.size(), 1u);
    EXPECT_EQ(log[0], 1);
}

TEST(PassManager, PassesRunInOrder) {
    std::vector<int> log;
    PassManager pm;
    pm.add<CountingPass>(log, 1)
      .add<CountingPass>(log, 2)
      .add<CountingPass>(log, 3);

    QFaultIRModule m{.level = IRLevel::LOGICAL};
    PassContext ctx{5};
    EXPECT_EQ(pm.run(m, ctx), PassResult::Success);
    ASSERT_EQ(log.size(), 3u);
    EXPECT_EQ(log[0], 1);
    EXPECT_EQ(log[1], 2);
    EXPECT_EQ(log[2], 3);
}

TEST(PassManager, FailureShortCircuits) {
    std::vector<int> log;
    PassManager pm;
    pm.add<CountingPass>(log, 1)
      .add<FailingPass>()
      .add<CountingPass>(log, 2);  // must NOT run

    QFaultIRModule m{.level = IRLevel::LOGICAL};
    PassContext ctx{5};
    EXPECT_EQ(pm.run(m, ctx), PassResult::Failure);
    ASSERT_EQ(log.size(), 1u);
    EXPECT_EQ(log[0], 1);
}

TEST(PassManager, MethodChainingReturnsRef) {
    PassManager pm;
    PassManager& ref = pm.add<FailingPass>();
    EXPECT_EQ(&ref, &pm);
}

TEST(PassManager, TimerIsStartedPerPass) {
    PassManager pm;
    pm.add<FailingPass>();

    QFaultIRModule m{.level = IRLevel::LOGICAL};
    PassContext ctx{5};
    pm.run(m, ctx);
    // After FailingPass, timer was started and stopped — duration is non-negative
    EXPECT_GE(ctx.lastPassDuration().count(), 0);
}

TEST(PassManager, PrintStatsContainsPassName) {
    std::vector<int> log;
    PassManager pm;
    pm.add<CountingPass>(log, 1);

    QFaultIRModule m{.level = IRLevel::LOGICAL};
    PassContext ctx{5};
    pm.run(m, ctx);

    std::ostringstream oss;
    pm.printStats(oss);
    EXPECT_NE(oss.str().find("CountingPass"), std::string::npos);
    EXPECT_NE(oss.str().find("Duration"), std::string::npos);
}

TEST(PassManager, PrintStatsBeforeRunPrintsEmptyTable) {
    PassManager pm;
    pm.add<FailingPass>();

    std::ostringstream oss;
    pm.printStats(oss);
    // Header still printed; no rows (no crash)
    EXPECT_NE(oss.str().find("Pass"), std::string::npos);
    EXPECT_EQ(pm.stats().size(), 0u);
}

TEST(PassManager, StatsAccumulatedPerPass) {
    std::vector<int> log;
    PassManager pm;
    pm.add<CountingPass>(log, 1)
      .add<CountingPass>(log, 2);

    QFaultIRModule m{.level = IRLevel::LOGICAL};
    PassContext ctx{5};
    pm.run(m, ctx);

    ASSERT_EQ(pm.stats().size(), 2u);
    EXPECT_EQ(pm.stats()[0].name, "CountingPass");
    EXPECT_EQ(pm.stats()[1].name, "CountingPass");
    EXPECT_GE(pm.stats()[0].duration.count(), 0);
}
