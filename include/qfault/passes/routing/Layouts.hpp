#pragma once

#include <qfault/passes/routing/AStar.hpp>

#include <cmath>
#include <cstddef>
#include <cstdint>

namespace qfault {

enum class LayoutChoice : std::uint8_t { Compact, Intermediate, Fast };

// Result returned by each layout builder.
struct Layout {
    PatchSpec    spec;
    std::size_t  tile_count{0}; // Litinski 2019 formula value (not grid area)
    LayoutChoice choice{LayoutChoice::Compact};
};

// ── Tile-count formulas (Litinski 2019 §5) ───────────────────────────────────
//
// Compact: n + n/2 + 3  (integer division — equals floor(1.5n)+3)
// Intermediate: 2n + 4   (body text; Fig. 13a "2.5n+4" is a known erratum — ADR-0019)
// Fast: 2n + ceil(sqrt(8n)) + 1  (Litinski 2019 §5.3 body text; confirmed against paper)

[[nodiscard]] constexpr std::size_t compactTileCount(std::size_t n) noexcept {
    return n + n / 2 + 3;
}

[[nodiscard]] constexpr std::size_t intermediateTileCount(std::size_t n) noexcept {
    return 2 * n + 4;
}

[[nodiscard]] std::size_t fastTileCount(std::size_t n) noexcept;

// ── Layout builders ───────────────────────────────────────────────────────────
//
// Each returns a Layout with a PatchSpec valid for the A* router.
// Data patches are placed at (0,0), (2,0), (4,0) … (2(n-1),0) — step=2 gives
// the mandatory ≥1 empty-tile gap between adjacent patches (routing rules).
// Boundary orientation: Z on N/S faces, X on E/W faces (ADR-0019 default).
// Factory tiles occupy the far-right cells in row 0 (TileState::Occupied).
// Row y=1 is entirely Empty — routing space for Z-type merges and detours.

[[nodiscard]] Layout compactLayout(std::size_t n);
[[nodiscard]] Layout intermediateLayout(std::size_t n);
[[nodiscard]] Layout fastLayout(std::size_t n);

// ── Layout selector ───────────────────────────────────────────────────────────
//
// Thresholds from GoSC performance targets:
//   compact      if t_depth ≤ num_qubits   (9τ/T; low T-count circuits)
//   intermediate if t_depth ≤ 4*num_qubits (5τ/T; balanced)
//   fast         otherwise                 (1τ/T; requires many factory units)
[[nodiscard]] LayoutChoice chooseLayout(std::size_t num_qubits,
                                        std::size_t t_depth) noexcept;

} // namespace qfault
