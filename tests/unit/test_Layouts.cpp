#include <qfault/passes/routing/Layouts.hpp>

#include <gtest/gtest.h>

#include <cmath>
#include <cstddef>

namespace qfault {
namespace {

// ── compactTileCount ─────────────────────────────────────────────────────────

TEST(Layouts, CompactTileCountFormula) {
    // n + n/2 + 3 (integer division = floor(1.5n) + 3)
    const std::pair<std::size_t, std::size_t> cases[] = {
        {1,   4},   // 1 + 0 + 3
        {2,   6},   // 2 + 1 + 3
        {4,   9},   // 4 + 2 + 3
        {8,  15},   // 8 + 4 + 3
        {10, 18},   // 10 + 5 + 3
        {16, 27},   // 16 + 8 + 3
        {100,153},  // 100 + 50 + 3
    };
    for (auto [n, expected] : cases) {
        EXPECT_EQ(compactTileCount(n), expected) << "n=" << n;
    }
}

// ── intermediateTileCount ────────────────────────────────────────────────────

TEST(Layouts, IntermediateTileCountFormula) {
    // 2n + 4 (body text; Fig. 13a "2.5n+4" is a known erratum — ADR-0019)
    const std::pair<std::size_t, std::size_t> cases[] = {
        {1,   6},
        {2,   8},
        {4,  12},
        {8,  20},
        {10, 24},  // stage 3 gate target: 2*10+4 = 24
        {16, 36},
        {100,204},
    };
    for (auto [n, expected] : cases) {
        EXPECT_EQ(intermediateTileCount(n), expected) << "n=" << n;
    }
}

// ── fastTileCount ────────────────────────────────────────────────────────────

TEST(Layouts, FastTileCountFormula) {
    // 2n + ceil(sqrt(8n)) + 1  — Litinski 2019 §5.3 (confirmed against paper)
    auto expected_fast = [](std::size_t n) -> std::size_t {
        const auto side = static_cast<std::size_t>(
            std::ceil(std::sqrt(8.0 * static_cast<double>(n))));
        return 2 * n + side + 1;
    };
    const std::size_t ns[] = {1, 2, 4, 8, 10, 16, 100};
    for (std::size_t n : ns) {
        EXPECT_EQ(fastTileCount(n), expected_fast(n)) << "n=" << n;
    }
}

TEST(Layouts, FastTileCountConcreteValues) {
    // Formula: 2n + ceil(sqrt(8n)) + 1  — confirmed Litinski 2019 §5.3
    // n=1:  2 + ceil(sqrt(8))  + 1 = 2 + 3 + 1 = 6
    // n=2:  4 + ceil(sqrt(16)) + 1 = 4 + 4 + 1 = 9
    // n=4:  8 + ceil(sqrt(32)) + 1 = 8 + 6 + 1 = 15
    // n=8:  16 + ceil(sqrt(64)) + 1 = 16 + 8 + 1 = 25
    // n=10: 20 + ceil(sqrt(80)) + 1 = 20 + 9 + 1 = 30
    // n=16: 32 + ceil(sqrt(128)) + 1 = 32 + 12 + 1 = 45
    EXPECT_EQ(fastTileCount(1),   6u);
    EXPECT_EQ(fastTileCount(2),   9u);
    EXPECT_EQ(fastTileCount(4),  15u);
    EXPECT_EQ(fastTileCount(8),  25u);
    EXPECT_EQ(fastTileCount(10), 30u);
    EXPECT_EQ(fastTileCount(16), 45u);
}

// ── Layout builders: tile_count field matches formula ────────────────────────

TEST(Layouts, CompactLayoutTileCountMatchesFormula) {
    const std::size_t ns[] = {1, 2, 4, 8, 10, 16, 100};
    for (std::size_t n : ns) {
        const Layout lay = compactLayout(n);
        EXPECT_EQ(lay.tile_count, compactTileCount(n)) << "n=" << n;
        EXPECT_EQ(lay.choice, LayoutChoice::Compact) << "n=" << n;
    }
}

TEST(Layouts, IntermediateLayoutTileCountMatchesFormula) {
    const std::size_t ns[] = {1, 2, 4, 8, 10, 16, 100};
    for (std::size_t n : ns) {
        const Layout lay = intermediateLayout(n);
        EXPECT_EQ(lay.tile_count, intermediateTileCount(n)) << "n=" << n;
        EXPECT_EQ(lay.choice, LayoutChoice::Intermediate) << "n=" << n;
    }
}

TEST(Layouts, FastLayoutTileCountMatchesFormula) {
    const std::size_t ns[] = {1, 2, 4, 8, 10, 16, 100};
    for (std::size_t n : ns) {
        const Layout lay = fastLayout(n);
        EXPECT_EQ(lay.tile_count, fastTileCount(n)) << "n=" << n;
        EXPECT_EQ(lay.choice, LayoutChoice::Fast) << "n=" << n;
    }
}

// ── Stage 3 gate target ───────────────────────────────────────────────────────

TEST(Layouts, IntermediateLayoutN10TileCountIs24) {
    // test_stage_3_gate.cpp predicate 9: intermediateLayout(10).tile_count == 24
    EXPECT_EQ(intermediateLayout(10).tile_count, 24u);
}

// ── Layout PatchSpec validity ─────────────────────────────────────────────────

TEST(Layouts, CompactLayoutHasCorrectDataPatches) {
    const Layout lay = compactLayout(5);
    const PatchSpec& spec = lay.spec;
    // Data patches at x=0,2,4,6,8 in row y=0
    for (int k = 0; k < 5; ++k) {
        EXPECT_EQ(spec.stateAt(2 * k, 0), TileState::DataPatch) << "k=" << k;
    }
    // Gaps between data patches are empty
    for (int k = 0; k < 4; ++k) {
        EXPECT_EQ(spec.stateAt(2 * k + 1, 0), TileState::Empty) << "gap k=" << k;
    }
    // Factory tiles (3 tiles)
    EXPECT_EQ(spec.stateAt(10, 0), TileState::Occupied);
    EXPECT_EQ(spec.stateAt(11, 0), TileState::Occupied);
    EXPECT_EQ(spec.stateAt(12, 0), TileState::Occupied);
    // Routing row is all empty
    for (int x = 0; x < spec.width; ++x) {
        EXPECT_EQ(spec.stateAt(x, 1), TileState::Empty) << "x=" << x;
    }
}

TEST(Layouts, IntermediateLayoutHasCorrectDataPatches) {
    const Layout lay = intermediateLayout(4);
    const PatchSpec& spec = lay.spec;
    // Data patches at x=0,2,4,6
    for (int k = 0; k < 4; ++k) {
        EXPECT_EQ(spec.stateAt(2 * k, 0), TileState::DataPatch) << "k=" << k;
    }
    // Factory (4 tiles) at x=8,9,10,11
    EXPECT_EQ(spec.stateAt(8,  0), TileState::Occupied);
    EXPECT_EQ(spec.stateAt(9,  0), TileState::Occupied);
    EXPECT_EQ(spec.stateAt(10, 0), TileState::Occupied);
    EXPECT_EQ(spec.stateAt(11, 0), TileState::Occupied);
}

TEST(Layouts, FastLayoutHasCorrectDataPatches) {
    const Layout lay = fastLayout(4);
    const PatchSpec& spec = lay.spec;
    // Data patches at x=0,2,4,6
    for (int k = 0; k < 4; ++k) {
        EXPECT_EQ(spec.stateAt(2 * k, 0), TileState::DataPatch) << "k=" << k;
    }
}

TEST(Layouts, DataPatchBoundariesSetCorrectly) {
    // Intermediate n=2: patches at (0,0) and (2,0)
    const Layout lay = intermediateLayout(2);
    const PatchSpec& spec = lay.spec;

    const NodeId id0 = spec.encode(0, 0);
    EXPECT_EQ(spec.boundaries[id0][static_cast<std::size_t>(Side::North)], BoundaryKind::Z);
    EXPECT_EQ(spec.boundaries[id0][static_cast<std::size_t>(Side::East)],  BoundaryKind::X);
    EXPECT_EQ(spec.boundaries[id0][static_cast<std::size_t>(Side::South)], BoundaryKind::Z);
    EXPECT_EQ(spec.boundaries[id0][static_cast<std::size_t>(Side::West)],  BoundaryKind::X);

    const NodeId id1 = spec.encode(2, 0);
    EXPECT_EQ(spec.boundaries[id1][static_cast<std::size_t>(Side::North)], BoundaryKind::Z);
    EXPECT_EQ(spec.boundaries[id1][static_cast<std::size_t>(Side::East)],  BoundaryKind::X);
    EXPECT_EQ(spec.boundaries[id1][static_cast<std::size_t>(Side::South)], BoundaryKind::Z);
    EXPECT_EQ(spec.boundaries[id1][static_cast<std::size_t>(Side::West)],  BoundaryKind::X);
}

// ── chooseLayout ──────────────────────────────────────────────────────────────

TEST(Layouts, ChooseLayoutCompactForLowTDepth) {
    EXPECT_EQ(chooseLayout(10, 0),  LayoutChoice::Compact);
    EXPECT_EQ(chooseLayout(10, 5),  LayoutChoice::Compact);
    EXPECT_EQ(chooseLayout(10, 10), LayoutChoice::Compact);
}

TEST(Layouts, ChooseLayoutIntermediateForModerateTDepth) {
    EXPECT_EQ(chooseLayout(10, 11), LayoutChoice::Intermediate);
    EXPECT_EQ(chooseLayout(10, 40), LayoutChoice::Intermediate);
}

TEST(Layouts, ChooseLayoutFastForHighTDepth) {
    EXPECT_EQ(chooseLayout(10, 41), LayoutChoice::Fast);
    EXPECT_EQ(chooseLayout(10, 1000), LayoutChoice::Fast);
}

TEST(Layouts, ChooseLayoutSingleQubits) {
    EXPECT_EQ(chooseLayout(1, 0), LayoutChoice::Compact);
    EXPECT_EQ(chooseLayout(1, 1), LayoutChoice::Compact);
    EXPECT_EQ(chooseLayout(1, 2), LayoutChoice::Intermediate);
    EXPECT_EQ(chooseLayout(1, 4), LayoutChoice::Intermediate);
    EXPECT_EQ(chooseLayout(1, 5), LayoutChoice::Fast);
}

// ── Ordering: compact ≤ intermediate ≤ fast tile counts ──────────────────────

TEST(Layouts, TileCountOrderingHoldsForAllTestN) {
    const std::size_t ns[] = {1, 2, 4, 8, 10, 16, 100};
    for (std::size_t n : ns) {
        EXPECT_LE(compactTileCount(n), intermediateTileCount(n)) << "n=" << n;
        EXPECT_LE(intermediateTileCount(n), fastTileCount(n)) << "n=" << n;
    }
}

} // namespace
} // namespace qfault
