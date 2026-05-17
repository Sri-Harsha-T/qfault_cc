#pragma once

#include <qfault/ir/MeasBasis.hpp>
#include <qfault/ir/PatchCoord.hpp>

#include <array>
#include <cstdint>
#include <limits>
#include <optional>
#include <vector>

namespace qfault {

// ── Fundamental types ─────────────────────────────────────────────────────────

// Flat tile index: NodeId = y * width + x.
// Never a pointer — pointer comparison breaks reproducibility under ASLR
// and explodes under ASan when vectors reallocate. (ADR-0006)
using NodeId = std::uint32_t;
constexpr NodeId kInvalidNode = std::numeric_limits<std::uint32_t>::max();

// Which face of a tile a boundary or route-entry is on.
enum class Side : std::uint8_t { North = 0, East = 1, South = 2, West = 3 };

// ADR-0019: dashed = X-rough, solid = Z-rough.
enum class BoundaryKind : std::uint8_t { X, Z };

// Occupation state of a tile in the patch grid.
enum class TileState : std::uint8_t { Empty, DataPatch, Occupied };

// ── Heap key for deterministic A* ────────────────────────────────────────────
// Comparison order: (f_score ASC) → (neg_g_score ASC = g DESC) → (insertion_seq ASC).
// This gives: lowest f wins; on tie, deeper search wins; on tie, earlier insert wins.
// All three levels are needed to make golden file comparisons reproducible.
struct AStarKey {
    std::int64_t  f_score;
    std::int64_t  neg_g_score;   // stored as -g so min-heap prefers larger g
    std::uint32_t insertion_seq; // monotone within one route() call
    [[nodiscard]] auto operator<=>(const AStarKey&) const noexcept = default;
};

// ── Static layout of the surface-code plane ──────────────────────────────────
struct PatchSpec {
    int width{0};
    int height{0};
    std::vector<TileState> tiles; // row-major: tiles[y * width + x]
    // boundaries[tile_index][static_cast<uint8_t>(Side)] = optional kind on that face.
    // DataPatch tiles carry their boundary kinds here; Empty tiles have nullopt.
    std::vector<std::array<std::optional<BoundaryKind>, 4>> boundaries;

    [[nodiscard]] bool       inBounds(int x, int y)  const noexcept;
    [[nodiscard]] NodeId     encode(int x, int y)     const noexcept;
    [[nodiscard]] PatchCoord decode(NodeId id)         const noexcept;
    [[nodiscard]] TileState  stateAt(int x, int y)    const noexcept;
};

// ── A single lattice surgery merge routing request ───────────────────────────
struct MergeRequest {
    PatchCoord src;
    Side       srcSide; // face of src that exposes the merging boundary
    PatchCoord dst;
    Side       dstSide; // face of dst that exposes the merging boundary
    MeasBasis  basis;   // X↔X or Z↔Z only — mixing X with Z is a logic error (ADR-0019)
};

// ── Result of routing one MergeRequest ───────────────────────────────────────
struct RouteResult {
    bool                    success{false};
    std::vector<PatchCoord> path; // ordered ancilla tiles; empty on failure
};

// ── A* configuration ─────────────────────────────────────────────────────────
struct AStarConfig {
    double        heuristic_weight{1.0}; // ε ∈ [1.0, 1.2]; 1.0 = admissible/optimal
    std::uint32_t max_nodes{1'000'000};  // safety cap; returns failure if exceeded
};

// ── Router ───────────────────────────────────────────────────────────────────
class AStarRouter {
public:
    explicit AStarRouter(const PatchSpec& spec, AStarConfig cfg = {});

    // Route one merge request. Returns success=false if the goal is unreachable.
    // Asserts if the boundary kinds do not match the request basis (programming error).
    [[nodiscard]] RouteResult route(const MergeRequest& req) const;

    // Route all requests independently (order does not affect individual results).
    [[nodiscard]] std::vector<RouteResult> routeAll(
        const std::vector<MergeRequest>& requests) const;

    // True iff both src and dst boundary kinds match the request basis.
    // Exposed for unit testing; route() asserts this before proceeding.
    [[nodiscard]] bool validateBoundary(const MergeRequest& req) const noexcept;

private:
    const PatchSpec& spec_;
    AStarConfig      cfg_;

    [[nodiscard]] static std::int64_t manhattan(PatchCoord a, PatchCoord b) noexcept;

    // Returns the empty tile adjacent to `patch` on `side`, or nullopt if
    // that tile is out of bounds or occupied.
    [[nodiscard]] std::optional<PatchCoord> entryTile(PatchCoord patch,
                                                       Side side) const noexcept;
};

// ── PatchSpec builder helpers ─────────────────────────────────────────────────

// Returns a flat grid where every tile is Empty and all boundaries are nullopt.
[[nodiscard]] PatchSpec makeEmptyGrid(int width, int height);

// Marks tile (x, y) as a DataPatch and sets its four boundary kinds.
// nullopt on a side means "open edge" (no logical boundary there).
void setDataPatch(PatchSpec& spec, int x, int y,
                  std::optional<BoundaryKind> north = std::nullopt,
                  std::optional<BoundaryKind> east  = std::nullopt,
                  std::optional<BoundaryKind> south = std::nullopt,
                  std::optional<BoundaryKind> west  = std::nullopt);

} // namespace qfault
