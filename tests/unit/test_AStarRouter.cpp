#include <qfault/passes/routing/AStar.hpp>

#include <gtest/gtest.h>

#include <cstdlib>

namespace qfault {
namespace {

// ── Shared grids ──────────────────────────────────────────────────────────────

// 3×3 grid with data patches at all four corners, Z boundaries on the faces
// that touch empty tiles:
//   (0,0)=DATA  (1,0)=EMPTY  (2,0)=DATA
//   (0,1)=EMPTY (1,1)=EMPTY  (2,1)=EMPTY
//   (0,2)=DATA  (1,2)=EMPTY  (2,2)=DATA
PatchSpec make3x3() {
    PatchSpec spec = makeEmptyGrid(3, 3);
    //                             N          E                S                W
    setDataPatch(spec, 0, 0, std::nullopt, BoundaryKind::Z, BoundaryKind::Z, std::nullopt);
    setDataPatch(spec, 2, 0, std::nullopt, std::nullopt,    BoundaryKind::Z, BoundaryKind::Z);
    setDataPatch(spec, 0, 2, BoundaryKind::Z, BoundaryKind::Z, std::nullopt, std::nullopt);
    setDataPatch(spec, 2, 2, BoundaryKind::Z, std::nullopt,    std::nullopt, BoundaryKind::Z);
    return spec;
}

// ── PatchSpec ─────────────────────────────────────────────────────────────────
TEST(PatchSpec, MakeEmptyGridAllTilesEmpty) {
    const PatchSpec spec = makeEmptyGrid(4, 6);
    for (int y = 0; y < 6; ++y)
        for (int x = 0; x < 4; ++x)
            EXPECT_EQ(spec.stateAt(x, y), TileState::Empty);
}

TEST(PatchSpec, EncodeDecodeRoundTrip) {
    const PatchSpec spec = makeEmptyGrid(7, 5);
    for (int y = 0; y < 5; ++y) {
        for (int x = 0; x < 7; ++x) {
            const NodeId id         = spec.encode(x, y);
            const PatchCoord back   = spec.decode(id);
            EXPECT_EQ(back.x, x);
            EXPECT_EQ(back.y, y);
        }
    }
}

TEST(PatchSpec, InBoundsCornerCases) {
    const PatchSpec spec = makeEmptyGrid(3, 3);
    EXPECT_TRUE(spec.inBounds(0, 0));
    EXPECT_TRUE(spec.inBounds(2, 2));
    EXPECT_FALSE(spec.inBounds(3, 0));
    EXPECT_FALSE(spec.inBounds(0, 3));
    EXPECT_FALSE(spec.inBounds(-1, 0));
    EXPECT_FALSE(spec.inBounds(0, -1));
}

TEST(PatchSpec, OutOfBoundsStatesOccupied) {
    const PatchSpec spec = makeEmptyGrid(3, 3);
    EXPECT_EQ(spec.stateAt(-1, 0), TileState::Occupied);
    EXPECT_EQ(spec.stateAt(0, -1), TileState::Occupied);
    EXPECT_EQ(spec.stateAt(3, 0), TileState::Occupied);
    EXPECT_EQ(spec.stateAt(0, 3), TileState::Occupied);
}

TEST(PatchSpec, SetDataPatchMarksCorrectTile) {
    PatchSpec spec = makeEmptyGrid(5, 5);
    setDataPatch(spec, 2, 3, BoundaryKind::X, std::nullopt, std::nullopt, BoundaryKind::Z);
    EXPECT_EQ(spec.stateAt(2, 3), TileState::DataPatch);
    EXPECT_EQ(spec.stateAt(2, 2), TileState::Empty);
    EXPECT_EQ(spec.stateAt(3, 3), TileState::Empty);

    const NodeId id = spec.encode(2, 3);
    EXPECT_EQ(spec.boundaries[id][static_cast<std::size_t>(Side::North)], BoundaryKind::X);
    EXPECT_EQ(spec.boundaries[id][static_cast<std::size_t>(Side::East)],  std::nullopt);
    EXPECT_EQ(spec.boundaries[id][static_cast<std::size_t>(Side::South)], std::nullopt);
    EXPECT_EQ(spec.boundaries[id][static_cast<std::size_t>(Side::West)],  BoundaryKind::Z);
}

// ── AStarRouter::validateBoundary ─────────────────────────────────────────────
TEST(AStarRouter, ValidateBoundaryZZPasses) {
    PatchSpec spec = makeEmptyGrid(3, 1);
    setDataPatch(spec, 0, 0, std::nullopt, BoundaryKind::Z, std::nullopt, std::nullopt);
    setDataPatch(spec, 2, 0, std::nullopt, std::nullopt,    std::nullopt, BoundaryKind::Z);
    const AStarRouter router{spec};
    EXPECT_TRUE(router.validateBoundary({
        .src     = {0, 0},
        .srcSide = Side::East,
        .dst     = {2, 0},
        .dstSide = Side::West,
        .basis   = MeasBasis::Z,
    }));
}

TEST(AStarRouter, ValidateBoundaryXXPasses) {
    PatchSpec spec = makeEmptyGrid(3, 1);
    setDataPatch(spec, 0, 0, std::nullopt, BoundaryKind::X, std::nullopt, std::nullopt);
    setDataPatch(spec, 2, 0, std::nullopt, std::nullopt,    std::nullopt, BoundaryKind::X);
    const AStarRouter router{spec};
    EXPECT_TRUE(router.validateBoundary({
        .src     = {0, 0},
        .srcSide = Side::East,
        .dst     = {2, 0},
        .dstSide = Side::West,
        .basis   = MeasBasis::X,
    }));
}

TEST(AStarRouter, ValidateBoundaryBasisMismatchFails) {
    // src has Z boundary but basis requests X — should return false.
    PatchSpec spec = makeEmptyGrid(3, 1);
    setDataPatch(spec, 0, 0, std::nullopt, BoundaryKind::Z, std::nullopt, std::nullopt);
    setDataPatch(spec, 2, 0, std::nullopt, std::nullopt,    std::nullopt, BoundaryKind::X);
    const AStarRouter router{spec};
    EXPECT_FALSE(router.validateBoundary({
        .src     = {0, 0},
        .srcSide = Side::East,
        .dst     = {2, 0},
        .dstSide = Side::West,
        .basis   = MeasBasis::X, // src is Z-rough — mismatch
    }));
}

TEST(AStarRouter, ValidateBoundaryNulloptFails) {
    // No boundary set on that side → false.
    PatchSpec spec = makeEmptyGrid(3, 1);
    setDataPatch(spec, 0, 0);  // all sides nullopt
    setDataPatch(spec, 2, 0, std::nullopt, std::nullopt, std::nullopt, BoundaryKind::Z);
    const AStarRouter router{spec};
    EXPECT_FALSE(router.validateBoundary({
        .src = {0, 0}, .srcSide = Side::East,
        .dst = {2, 0}, .dstSide = Side::West,
        .basis = MeasBasis::Z,
    }));
}

// ── AStarRouter::route — basic cases ──────────────────────────────────────────
TEST(AStarRouter, TrivialRouteSingleSharedTile) {
    // Patches at x=0 and x=2 both connect through x=1 — start equals goal.
    PatchSpec spec = makeEmptyGrid(3, 1);
    setDataPatch(spec, 0, 0, std::nullopt, BoundaryKind::Z, std::nullopt, std::nullopt);
    setDataPatch(spec, 2, 0, std::nullopt, std::nullopt,    std::nullopt, BoundaryKind::Z);
    const AStarRouter router{spec};
    const auto result = router.route({
        .src = {0,0}, .srcSide = Side::East,
        .dst = {2,0}, .dstSide = Side::West,
        .basis = MeasBasis::Z,
    });
    EXPECT_TRUE(result.success);
    ASSERT_EQ(result.path.size(), 1u);
    EXPECT_EQ(result.path[0], (PatchCoord{1, 0}));
}

TEST(AStarRouter, RouteHorizontalStrip) {
    // DATA  EMPTY  EMPTY  EMPTY  DATA  →  path length 3
    PatchSpec spec = makeEmptyGrid(5, 1);
    setDataPatch(spec, 0, 0, std::nullopt, BoundaryKind::Z, std::nullopt, std::nullopt);
    setDataPatch(spec, 4, 0, std::nullopt, std::nullopt,    std::nullopt, BoundaryKind::Z);
    const AStarRouter router{spec};
    const auto result = router.route({
        .src = {0,0}, .srcSide = Side::East,
        .dst = {4,0}, .dstSide = Side::West,
        .basis = MeasBasis::Z,
    });
    EXPECT_TRUE(result.success);
    ASSERT_EQ(result.path.size(), 3u);
    EXPECT_EQ(result.path[0], (PatchCoord{1, 0}));
    EXPECT_EQ(result.path[1], (PatchCoord{2, 0}));
    EXPECT_EQ(result.path[2], (PatchCoord{3, 0}));
}

TEST(AStarRouter, RouteAroundCornerIn3x3) {
    // src=(0,0) East → entry=(1,0); dst=(2,2) West → entry=(1,2)
    // Only feasible shortest path: (1,0)→(1,1)→(1,2)
    const PatchSpec spec = make3x3();
    const AStarRouter router{spec};
    const auto result = router.route({
        .src = {0,0}, .srcSide = Side::East,
        .dst = {2,2}, .dstSide = Side::West,
        .basis = MeasBasis::Z,
    });
    EXPECT_TRUE(result.success);
    ASSERT_EQ(result.path.size(), 3u);
    EXPECT_EQ(result.path.front(), (PatchCoord{1, 0}));
    EXPECT_EQ(result.path.back(),  (PatchCoord{1, 2}));
}

TEST(AStarRouter, RouteVerticalStrip) {
    // Patches at (1,0) South and (1,4) North; path goes through (1,1),(1,2),(1,3)
    PatchSpec spec = makeEmptyGrid(3, 5);
    setDataPatch(spec, 1, 0, std::nullopt, std::nullopt, BoundaryKind::Z, std::nullopt);
    setDataPatch(spec, 1, 4, BoundaryKind::Z, std::nullopt, std::nullopt, std::nullopt);
    const AStarRouter router{spec};
    const auto result = router.route({
        .src = {1,0}, .srcSide = Side::South,
        .dst = {1,4}, .dstSide = Side::North,
        .basis = MeasBasis::Z,
    });
    EXPECT_TRUE(result.success);
    ASSERT_EQ(result.path.size(), 3u);
    EXPECT_EQ(result.path[0], (PatchCoord{1, 1}));
    EXPECT_EQ(result.path[1], (PatchCoord{1, 2}));
    EXPECT_EQ(result.path[2], (PatchCoord{1, 3}));
}

TEST(AStarRouter, UnreachableGoalReturnsFailure) {
    // Block the only path: DATA OCCUPIED DATA
    PatchSpec spec = makeEmptyGrid(3, 1);
    setDataPatch(spec, 0, 0, std::nullopt, BoundaryKind::Z, std::nullopt, std::nullopt);
    setDataPatch(spec, 1, 0); // wall — blocks every route
    setDataPatch(spec, 2, 0, std::nullopt, std::nullopt,    std::nullopt, BoundaryKind::Z);
    const AStarRouter router{spec};
    const auto result = router.route({
        .src = {0,0}, .srcSide = Side::East,
        .dst = {2,0}, .dstSide = Side::West,
        .basis = MeasBasis::Z,
    });
    EXPECT_FALSE(result.success);
    EXPECT_TRUE(result.path.empty());
}

TEST(AStarRouter, NoValidEntryTileReturnsFailure) {
    // The tile adjacent to src on srcSide is itself a DataPatch — no entry.
    PatchSpec spec = makeEmptyGrid(4, 1);
    setDataPatch(spec, 0, 0, std::nullopt, BoundaryKind::Z, std::nullopt, std::nullopt);
    setDataPatch(spec, 1, 0); // blocks entry for src's East side
    setDataPatch(spec, 3, 0, std::nullopt, std::nullopt,    std::nullopt, BoundaryKind::Z);
    const AStarRouter router{spec};
    const auto result = router.route({
        .src = {0,0}, .srcSide = Side::East,
        .dst = {3,0}, .dstSide = Side::West,
        .basis = MeasBasis::Z,
    });
    EXPECT_FALSE(result.success);
}

TEST(AStarRouter, PathIsContiguous) {
    // Every consecutive pair of tiles in the path must be Manhattan-adjacent.
    PatchSpec spec = makeEmptyGrid(10, 10);
    setDataPatch(spec, 0, 0, std::nullopt, BoundaryKind::Z, std::nullopt, std::nullopt);
    setDataPatch(spec, 9, 9, std::nullopt, std::nullopt,    std::nullopt, BoundaryKind::Z);
    const AStarRouter router{spec};
    const auto result = router.route({
        .src = {0,0}, .srcSide = Side::East,
        .dst = {9,9}, .dstSide = Side::West,
        .basis = MeasBasis::Z,
    });
    ASSERT_TRUE(result.success);
    for (std::size_t i = 1; i < result.path.size(); ++i) {
        const int dx = std::abs(result.path[i].x - result.path[i - 1].x);
        const int dy = std::abs(result.path[i].y - result.path[i - 1].y);
        EXPECT_EQ(dx + dy, 1) << "path not contiguous at index " << i;
    }
}

TEST(AStarRouter, RouteDeterministicAcrossRepeatedCalls) {
    PatchSpec spec = makeEmptyGrid(10, 10);
    setDataPatch(spec, 0, 5, std::nullopt, BoundaryKind::Z, std::nullopt, std::nullopt);
    setDataPatch(spec, 9, 5, std::nullopt, std::nullopt,    std::nullopt, BoundaryKind::Z);
    const AStarRouter router{spec};
    const MergeRequest req{
        .src = {0,5}, .srcSide = Side::East,
        .dst = {9,5}, .dstSide = Side::West,
        .basis = MeasBasis::Z,
    };
    const auto r1 = router.route(req);
    const auto r2 = router.route(req);
    ASSERT_TRUE(r1.success);
    ASSERT_TRUE(r2.success);
    EXPECT_EQ(r1.path, r2.path);
}

TEST(AStarRouter, RouteAllReturnsOneResultPerRequest) {
    PatchSpec spec = makeEmptyGrid(5, 1);
    setDataPatch(spec, 0, 0, std::nullopt, BoundaryKind::Z, std::nullopt, std::nullopt);
    setDataPatch(spec, 4, 0, std::nullopt, std::nullopt,    std::nullopt, BoundaryKind::Z);
    const AStarRouter router{spec};
    const std::vector<MergeRequest> reqs{{
        .src = {0,0}, .srcSide = Side::East,
        .dst = {4,0}, .dstSide = Side::West,
        .basis = MeasBasis::Z,
    }};
    const auto results = router.routeAll(reqs);
    ASSERT_EQ(results.size(), 1u);
    EXPECT_TRUE(results[0].success);
}

TEST(AStarRouter, RouteAllEmptyRequestsReturnsEmpty) {
    const PatchSpec spec = makeEmptyGrid(3, 3);
    const AStarRouter router{spec};
    const auto results = router.routeAll({});
    EXPECT_TRUE(results.empty());
}

TEST(AStarRouter, PathStartsAdjacentToSrc) {
    // The first tile in the path must be the neighbor of src on srcSide.
    PatchSpec spec = makeEmptyGrid(7, 1);
    setDataPatch(spec, 0, 0, std::nullopt, BoundaryKind::Z, std::nullopt, std::nullopt);
    setDataPatch(spec, 6, 0, std::nullopt, std::nullopt,    std::nullopt, BoundaryKind::Z);
    const AStarRouter router{spec};
    const auto result = router.route({
        .src = {0,0}, .srcSide = Side::East,
        .dst = {6,0}, .dstSide = Side::West,
        .basis = MeasBasis::Z,
    });
    ASSERT_TRUE(result.success);
    // src=(0,0), srcSide=East → entry tile must be (1,0)
    EXPECT_EQ(result.path.front(), (PatchCoord{1, 0}));
    // dst=(6,0), dstSide=West → exit tile must be (5,0)
    EXPECT_EQ(result.path.back(), (PatchCoord{5, 0}));
}

TEST(AStarRouter, MaxNodesCapReturnsFailure) {
    // Set max_nodes=1 so the router gives up immediately on a non-trivial route.
    PatchSpec spec = makeEmptyGrid(5, 1);
    setDataPatch(spec, 0, 0, std::nullopt, BoundaryKind::Z, std::nullopt, std::nullopt);
    setDataPatch(spec, 4, 0, std::nullopt, std::nullopt,    std::nullopt, BoundaryKind::Z);
    const AStarRouter router{spec, AStarConfig{.heuristic_weight = 1.0, .max_nodes = 1u}};
    const auto result = router.route({
        .src = {0,0}, .srcSide = Side::East,
        .dst = {4,0}, .dstSide = Side::West,
        .basis = MeasBasis::Z,
    });
    EXPECT_FALSE(result.success);
}

TEST(AStarRouter, WeightedHeuristicFindsPath) {
    // ε=1.1 still finds a valid (possibly suboptimal) path.
    PatchSpec spec = makeEmptyGrid(10, 10);
    setDataPatch(spec, 0, 0, std::nullopt, BoundaryKind::Z, std::nullopt, std::nullopt);
    setDataPatch(spec, 9, 9, std::nullopt, std::nullopt,    std::nullopt, BoundaryKind::Z);
    const AStarRouter router{spec, AStarConfig{.heuristic_weight = 1.1}};
    const auto result = router.route({
        .src = {0,0}, .srcSide = Side::East,
        .dst = {9,9}, .dstSide = Side::West,
        .basis = MeasBasis::Z,
    });
    EXPECT_TRUE(result.success);
    EXPECT_FALSE(result.path.empty());
}

} // namespace
} // namespace qfault
