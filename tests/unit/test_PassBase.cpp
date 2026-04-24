#include <qfault/ir/IRLevel.hpp>
#include <qfault/ir/QFaultIRModule.hpp>
#include <qfault/passes/PassBase.hpp>
#include <qfault/passes/PassContext.hpp>

#include <gtest/gtest.h>
#include <memory>
#include <string_view>

using namespace qfault;

// Minimal concrete pass for testing the interface
class StubPass : public PassBase {
public:
    [[nodiscard]] std::string_view name() const override { return "StubPass"; }
    [[nodiscard]] IRLevel requiredLevel() const override { return IRLevel::LOGICAL; }
    PassResult run(QFaultIRModule& module, PassContext& /*ctx*/) override {
        module.assertLevel(requiredLevel());
        runCount_++;
        return PassResult::Success;
    }
    int runCount() const { return runCount_; }

private:
    int runCount_{0};
};

TEST(PassBase, ConcretePassStoredInUniquePtr) {
    std::unique_ptr<PassBase> pass = std::make_unique<StubPass>();
    EXPECT_EQ(pass->name(), "StubPass");
    EXPECT_EQ(pass->requiredLevel(), IRLevel::LOGICAL);
}

TEST(PassBase, RunReturnsSuccess) {
    StubPass pass;
    QFaultIRModule module{.level = IRLevel::LOGICAL};
    PassContext ctx{5};

    EXPECT_EQ(pass.run(module, ctx), PassResult::Success);
    EXPECT_EQ(pass.runCount(), 1);
}

TEST(PassBase, RunThrowsOnWrongLevel) {
    StubPass pass;
    QFaultIRModule module{.level = IRLevel::PHYSICAL};
    PassContext ctx{5};

    EXPECT_THROW(pass.run(module, ctx), std::logic_error);
}

TEST(PassBase, PassResultValuesDistinct) {
    EXPECT_NE(PassResult::Success, PassResult::Failure);
}
