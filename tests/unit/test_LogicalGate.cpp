#include <qfault/ir/GateKind.hpp>
#include <qfault/ir/LogicalGate.hpp>
#include <qfault/ir/LogicalQubit.hpp>

#include <gtest/gtest.h>

using namespace qfault;

TEST(LogicalQubit, ConstructsWithNameAndIndex) {
    LogicalQubit q{.name = "q", .index = 2};
    EXPECT_EQ(q.name, "q");
    EXPECT_EQ(q.index, 2u);
}

TEST(LogicalQubit, EqualityOnMatchingFields) {
    LogicalQubit a{.name = "q", .index = 0};
    LogicalQubit b{.name = "q", .index = 0};
    EXPECT_EQ(a, b);
}

TEST(LogicalQubit, InequalityOnDifferentName) {
    LogicalQubit a{.name = "q", .index = 0};
    LogicalQubit b{.name = "r", .index = 0};
    EXPECT_NE(a, b);
}

TEST(LogicalQubit, InequalityOnDifferentIndex) {
    LogicalQubit a{.name = "q", .index = 0};
    LogicalQubit b{.name = "q", .index = 1};
    EXPECT_NE(a, b);
}

TEST(LogicalGate, SingleQubitClifford) {
    LogicalQubit q{.name = "q", .index = 0};
    LogicalGate g{.kind = GateKind::H, .operands = {q}, .angle = std::nullopt};
    EXPECT_EQ(g.kind, GateKind::H);
    EXPECT_EQ(g.operands.size(), 1u);
    EXPECT_EQ(g.operands[0], q);
    EXPECT_FALSE(g.angle.has_value());
}

TEST(LogicalGate, TGate) {
    LogicalQubit q{.name = "q", .index = 1};
    LogicalGate g{.kind = GateKind::T, .operands = {q}, .angle = std::nullopt};
    EXPECT_EQ(g.kind, GateKind::T);
    EXPECT_FALSE(g.angle.has_value());
}

TEST(LogicalGate, TwoQubitCX) {
    LogicalQubit ctrl{.name = "q", .index = 0};
    LogicalQubit tgt{.name = "q", .index = 1};
    LogicalGate g{.kind = GateKind::CX, .operands = {ctrl, tgt}, .angle = std::nullopt};
    EXPECT_EQ(g.kind, GateKind::CX);
    EXPECT_EQ(g.operands.size(), 2u);
}

TEST(LogicalGate, RotationGateHasAngle) {
    LogicalQubit q{.name = "q", .index = 0};
    LogicalGate g{.kind = GateKind::RZ, .operands = {q}, .angle = 1.5707963};
    EXPECT_TRUE(g.angle.has_value());
    EXPECT_DOUBLE_EQ(*g.angle, 1.5707963);
}

TEST(LogicalGate, EqualityOnIdenticalGates) {
    LogicalQubit q{.name = "q", .index = 0};
    LogicalGate a{.kind = GateKind::S, .operands = {q}, .angle = std::nullopt};
    LogicalGate b{.kind = GateKind::S, .operands = {q}, .angle = std::nullopt};
    EXPECT_EQ(a, b);
}

TEST(LogicalGate, InequalityOnDifferentKind) {
    LogicalQubit q{.name = "q", .index = 0};
    LogicalGate a{.kind = GateKind::S, .operands = {q}, .angle = std::nullopt};
    LogicalGate b{.kind = GateKind::Sdg, .operands = {q}, .angle = std::nullopt};
    EXPECT_NE(a, b);
}

TEST(GateKind, AllValuesDistinct) {
    // Smoke test: enum values compile and differ
    EXPECT_NE(GateKind::H, GateKind::X);
    EXPECT_NE(GateKind::T, GateKind::Tdg);
    EXPECT_NE(GateKind::S, GateKind::Sdg);
    EXPECT_NE(GateKind::CX, GateKind::CZ);
    EXPECT_NE(GateKind::RZ, GateKind::U);
}
