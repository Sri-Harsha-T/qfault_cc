#include <qfault/passes/synthesis/SynthesisProvider.hpp>
#include <qfault/ir/GateKind.hpp>

#include <gtest/gtest.h>

#include <numbers>
#include <string_view>
#include <vector>

namespace {

// Minimal mock that satisfies the concept.
struct MockProvider {
    [[nodiscard]] std::vector<qfault::GateKind> synthesise(double /*angle*/,
                                                            double /*eps*/) {
        return {qfault::GateKind::T};
    }
    [[nodiscard]] std::string_view name() const { return "MockProvider"; }
};

// A type that is missing both required methods.
struct NotAProvider {};

// A type that has synthesise but is missing name().
struct MissingName {
    [[nodiscard]] std::vector<qfault::GateKind> synthesise(double, double) {
        return {};
    }
};

// A type that has name() but is missing synthesise().
struct MissingSynthesize {
    [[nodiscard]] std::string_view name() const { return "x"; }
};

static_assert(qfault::SynthesisProvider<MockProvider>,
              "MockProvider must satisfy SynthesisProvider");
static_assert(!qfault::SynthesisProvider<NotAProvider>,
              "NotAProvider must NOT satisfy SynthesisProvider");
static_assert(!qfault::SynthesisProvider<MissingName>,
              "MissingName must NOT satisfy SynthesisProvider");
static_assert(!qfault::SynthesisProvider<MissingSynthesize>,
              "MissingSynthesize must NOT satisfy SynthesisProvider");
static_assert(!qfault::SynthesisProvider<int>,
              "int must NOT satisfy SynthesisProvider");

} // namespace

TEST(SynthesisProvider, MockProviderSatisfiesConcept) {
    MockProvider p;
    auto seq = p.synthesise(std::numbers::pi / 4.0, 1e-10);
    EXPECT_FALSE(seq.empty());
    EXPECT_EQ(p.name(), "MockProvider");
}

TEST(SynthesisProvider, MockProviderReturnsGateKindSequence) {
    MockProvider p;
    auto seq = p.synthesise(0.0, 1e-6);
    for (auto g : seq) {
        // All returned values are valid GateKind enumerators — just cast-check.
        const auto val = static_cast<int>(g);
        EXPECT_GE(val, static_cast<int>(qfault::GateKind::H));
        EXPECT_LE(val, static_cast<int>(qfault::GateKind::U));
    }
}
