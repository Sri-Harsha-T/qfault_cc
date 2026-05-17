#include <qfault/passes/routing/Layouts.hpp>

#include <cassert>
#include <cmath>

namespace qfault {

std::size_t fastTileCount(std::size_t n) noexcept {
    // ceil(sqrt(8n)) — Litinski 2019 §5.3 fast block.
    // Exact when 8n is a perfect square; ceil rounds up otherwise.
    const auto side = static_cast<std::size_t>(
        std::ceil(std::sqrt(8.0 * static_cast<double>(n))));
    return 2 * n + side + 1;
}

// Place n data patches at (2k, 0) for k = 0..n-1.
// Boundaries: Z on North/South (merges via row-1 routing space),
//             X on East/West   (merges via the even-x gaps in row 0).
// Step of 2 guarantees ≥1 empty tile between adjacent patches (routing rules).
static void placeDataRow(PatchSpec& spec, std::size_t n) {
    for (std::size_t k = 0; k < n; ++k) {
        setDataPatch(spec, static_cast<int>(2 * k), 0,
                     BoundaryKind::Z,   // north → routing row above
                     BoundaryKind::X,   // east  → gap tile
                     BoundaryKind::Z,   // south → below grid edge (treated Occupied)
                     BoundaryKind::X);  // west  → gap tile
    }
}

// Mark tiles in row y=0 from x=x_start to x=width-1 as Occupied (factory block).
static void placeFactory(PatchSpec& spec, int x_start) {
    const int w = spec.width;
    for (int fx = x_start; fx < w; ++fx) {
        spec.tiles[static_cast<std::size_t>(fx)] = TileState::Occupied;
    }
}

Layout compactLayout(std::size_t n) {
    assert(n >= 1);
    const std::size_t tc = compactTileCount(n); // n + n/2 + 3

    // Grid: width = 2n+3, height = 2.
    // Row y=0: data patches at x=0,2,…,2(n-1); factory (3 tiles) at x=2n,2n+1,2n+2.
    // Row y=1: all Empty (routing space).
    const int w = static_cast<int>(2 * n + 3);
    PatchSpec spec = makeEmptyGrid(w, 2);
    placeDataRow(spec, n);
    placeFactory(spec, static_cast<int>(2 * n));

    return Layout{std::move(spec), tc, LayoutChoice::Compact};
}

Layout intermediateLayout(std::size_t n) {
    assert(n >= 1);
    const std::size_t tc = intermediateTileCount(n); // 2n + 4
    // Fig. 13a caption "2.5n+4" is a known erratum; body text says 2n+4 (ADR-0019).

    // Grid: width = 2n+4, height = 2.
    // Row y=0: data patches at x=0,2,…,2(n-1); factory (4 tiles) at x=2n..2n+3.
    // Row y=1: all Empty (routing space).
    const int w = static_cast<int>(2 * n + 4);
    PatchSpec spec = makeEmptyGrid(w, 2);
    placeDataRow(spec, n);
    placeFactory(spec, static_cast<int>(2 * n));

    return Layout{std::move(spec), tc, LayoutChoice::Intermediate};
}

Layout fastLayout(std::size_t n) {
    assert(n >= 1);
    const std::size_t tc = fastTileCount(n); // 2n + ceil(sqrt(2n)) + 1

    const auto side = static_cast<int>(
        std::ceil(std::sqrt(8.0 * static_cast<double>(n))));

    // Grid: width = 2n + side + 1, height = 2.
    // Row y=0: data patches at x=0,2,…,2(n-1); factory (side+1 tiles) at far right.
    // Row y=1: all Empty (routing space).
    const int w = static_cast<int>(2 * n) + side + 1;
    PatchSpec spec = makeEmptyGrid(w, 2);
    placeDataRow(spec, n);
    placeFactory(spec, static_cast<int>(2 * n));

    return Layout{std::move(spec), tc, LayoutChoice::Fast};
}

LayoutChoice chooseLayout(std::size_t num_qubits, std::size_t t_depth) noexcept {
    if (t_depth <= num_qubits)       return LayoutChoice::Compact;
    if (t_depth <= 4 * num_qubits)   return LayoutChoice::Intermediate;
    return LayoutChoice::Fast;
}

} // namespace qfault
