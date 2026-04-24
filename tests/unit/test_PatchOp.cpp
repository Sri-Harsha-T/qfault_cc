#include <qfault/ir/MeasBasis.hpp>
#include <qfault/ir/PatchCoord.hpp>
#include <qfault/ir/PatchOp.hpp>
#include <qfault/ir/PatchOpKind.hpp>

#include <gtest/gtest.h>
#include <map>

using namespace qfault;

TEST(PatchCoord, ConstructsWithXY) {
    PatchCoord p{.x = 3, .y = 7};
    EXPECT_EQ(p.x, 3);
    EXPECT_EQ(p.y, 7);
}

TEST(PatchCoord, Equality) {
    EXPECT_EQ((PatchCoord{.x = 1, .y = 2}), (PatchCoord{.x = 1, .y = 2}));
    EXPECT_NE((PatchCoord{.x = 1, .y = 2}), (PatchCoord{.x = 1, .y = 3}));
    EXPECT_NE((PatchCoord{.x = 0, .y = 2}), (PatchCoord{.x = 1, .y = 2}));
}

TEST(PatchCoord, OrderingForMapKey) {
    // Verify PatchCoord can be used as a std::map key (requires operator<).
    std::map<PatchCoord, int> grid;
    grid[{.x = 0, .y = 0}] = 1;
    grid[{.x = 1, .y = 0}] = 2;
    grid[{.x = 0, .y = 1}] = 3;
    EXPECT_EQ(grid.size(), 3u);
    EXPECT_EQ(grid.at({.x = 1, .y = 0}), 2);
}

TEST(PatchCoord, OrderingConsistency) {
    PatchCoord a{.x = 0, .y = 5};
    PatchCoord b{.x = 1, .y = 0};
    // x takes priority in ordering
    EXPECT_TRUE(a < b);
    EXPECT_FALSE(b < a);
}

TEST(MeasBasis, ValuesDistinct) {
    EXPECT_NE(MeasBasis::X, MeasBasis::Z);
}

TEST(PatchOpKind, AllValuesDistinct) {
    EXPECT_NE(PatchOpKind::MERGE, PatchOpKind::SPLIT);
    EXPECT_NE(PatchOpKind::MEASURE, PatchOpKind::IDLE);
}

TEST(PatchOp, MergeOp) {
    PatchOp op{
        .kind = PatchOpKind::MERGE,
        .patches = {{.x = 0, .y = 0}, {.x = 1, .y = 0}},
        .basis = MeasBasis::X,
        .timeStep = 3,
    };
    EXPECT_EQ(op.kind, PatchOpKind::MERGE);
    EXPECT_EQ(op.patches.size(), 2u);
    EXPECT_EQ(op.basis, MeasBasis::X);
    EXPECT_EQ(op.timeStep, 3u);
}

TEST(PatchOp, IdleOp) {
    PatchOp op{.kind = PatchOpKind::IDLE, .patches = {{.x = 2, .y = 2}}};
    EXPECT_EQ(op.kind, PatchOpKind::IDLE);
    EXPECT_EQ(op.timeStep, 0u);
}

TEST(PatchOp, EqualityOnIdentical) {
    PatchOp a{
        .kind = PatchOpKind::MEASURE,
        .patches = {{.x = 0, .y = 0}},
        .basis = MeasBasis::Z,
        .timeStep = 1,
    };
    PatchOp b = a;
    EXPECT_EQ(a, b);
}

TEST(PatchOp, InequalityOnDifferentTimeStep) {
    PatchOp a{.kind = PatchOpKind::IDLE, .patches = {{.x = 0, .y = 0}}, .timeStep = 0};
    PatchOp b{.kind = PatchOpKind::IDLE, .patches = {{.x = 0, .y = 0}}, .timeStep = 1};
    EXPECT_NE(a, b);
}
