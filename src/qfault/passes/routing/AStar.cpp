#include <qfault/passes/routing/AStar.hpp>

#include <algorithm>
#include <cassert>
#include <cmath>
#include <cstdlib>
#include <queue>
#include <ranges>
#include <utility>

namespace qfault {

// ── Direction offset table indexed by Side enum value ────────────────────────
// North=0:(0,-1)  East=1:(1,0)  South=2:(0,1)  West=3:(-1,0)
static constexpr std::array<std::pair<int, int>, 4> kDirOffset{{
    { 0, -1},
    { 1,  0},
    { 0,  1},
    {-1,  0},
}};

// ── PatchSpec ────────────────────────────────────────────────────────────────
bool PatchSpec::inBounds(int x, int y) const noexcept {
    return x >= 0 && x < width && y >= 0 && y < height;
}

NodeId PatchSpec::encode(int x, int y) const noexcept {
    assert(inBounds(x, y) && "encode: tile coordinates out of bounds");
    return static_cast<NodeId>(y * width + x);
}

PatchCoord PatchSpec::decode(NodeId id) const noexcept {
    const int idx = static_cast<int>(id);
    return PatchCoord{idx % width, idx / width};
}

TileState PatchSpec::stateAt(int x, int y) const noexcept {
    if (!inBounds(x, y)) return TileState::Occupied;
    return tiles[static_cast<std::size_t>(y * width + x)];
}

// ── Builder helpers ───────────────────────────────────────────────────────────
PatchSpec makeEmptyGrid(int width, int height) {
    const std::size_t n = static_cast<std::size_t>(width * height);
    return PatchSpec{
        .width      = width,
        .height     = height,
        .tiles      = std::vector<TileState>(n, TileState::Empty),
        .boundaries = std::vector<std::array<std::optional<BoundaryKind>, 4>>(
            n, std::array<std::optional<BoundaryKind>, 4>{}),
    };
}

void setDataPatch(PatchSpec& spec, int x, int y,
                  std::optional<BoundaryKind> north,
                  std::optional<BoundaryKind> east,
                  std::optional<BoundaryKind> south,
                  std::optional<BoundaryKind> west)
{
    assert(spec.inBounds(x, y) && "setDataPatch: tile out of bounds");
    const std::size_t idx = static_cast<std::size_t>(spec.encode(x, y));
    spec.tiles[idx]                                              = TileState::DataPatch;
    spec.boundaries[idx][static_cast<std::size_t>(Side::North)] = north;
    spec.boundaries[idx][static_cast<std::size_t>(Side::East)]  = east;
    spec.boundaries[idx][static_cast<std::size_t>(Side::South)] = south;
    spec.boundaries[idx][static_cast<std::size_t>(Side::West)]  = west;
}

// ── AStarRouter ──────────────────────────────────────────────────────────────
AStarRouter::AStarRouter(const PatchSpec& spec, AStarConfig cfg)
    : spec_(spec), cfg_(cfg) {}

std::int64_t AStarRouter::manhattan(PatchCoord a, PatchCoord b) noexcept {
    return static_cast<std::int64_t>(std::abs(a.x - b.x)) +
           static_cast<std::int64_t>(std::abs(a.y - b.y));
}

std::optional<PatchCoord> AStarRouter::entryTile(PatchCoord patch,
                                                   Side side) const noexcept {
    const auto [dx, dy] = kDirOffset[static_cast<std::size_t>(side)];
    const int nx = patch.x + dx;
    const int ny = patch.y + dy;
    if (!spec_.inBounds(nx, ny)) return std::nullopt;
    if (spec_.stateAt(nx, ny) != TileState::Empty) return std::nullopt;
    return PatchCoord{nx, ny};
}

bool AStarRouter::validateBoundary(const MergeRequest& req) const noexcept {
    const BoundaryKind expected =
        (req.basis == MeasBasis::X) ? BoundaryKind::X : BoundaryKind::Z;

    auto check = [&](PatchCoord patch, Side side) -> bool {
        if (!spec_.inBounds(patch.x, patch.y)) return false;
        const NodeId nid   = spec_.encode(patch.x, patch.y);
        const auto&  bnd   = spec_.boundaries[nid][static_cast<std::size_t>(side)];
        return bnd.has_value() && bnd.value() == expected;
    };

    return check(req.src, req.srcSide) && check(req.dst, req.dstSide);
}

RouteResult AStarRouter::route(const MergeRequest& req) const {
    // Mixing X↔Z boundaries is a logic error, not a recoverable runtime error (ADR-0019).
    assert(validateBoundary(req) &&
           "route: boundary kind mismatch — merge must be X↔X or Z↔Z");

    RouteResult result;

    const auto maybeStart = entryTile(req.src, req.srcSide);
    const auto maybeGoal  = entryTile(req.dst, req.dstSide);
    if (!maybeStart.has_value() || !maybeGoal.has_value()) return result;

    const PatchCoord startCoord = *maybeStart;
    const PatchCoord goalCoord  = *maybeGoal;

    const NodeId startId = spec_.encode(startCoord.x, startCoord.y);
    const NodeId goalId  = spec_.encode(goalCoord.x,  goalCoord.y);

    // Trivial: both patches share the same adjacent ancilla tile.
    if (startId == goalId) {
        result.success = true;
        result.path    = {startCoord};
        return result;
    }

    const std::size_t numTiles =
        static_cast<std::size_t>(spec_.width) *
        static_cast<std::size_t>(spec_.height);

    // Closed list: one byte per tile — cache-friendly, sanitizer-clean (ADR-0006).
    std::vector<std::uint8_t> closed(numTiles, 0);

    // Best g_score known per tile; unvisited tiles start at max.
    std::vector<std::int64_t> gScore(numTiles,
                                     std::numeric_limits<std::int64_t>::max());

    // For path reconstruction: cameFrom[node] = predecessor.
    std::vector<NodeId> cameFrom(numTiles, kInvalidNode);

    // Min-heap keyed by AStarKey for deterministic, ASLR-independent ordering.
    using HeapEntry = std::pair<AStarKey, NodeId>;
    std::priority_queue<HeapEntry,
                        std::vector<HeapEntry>,
                        std::greater<HeapEntry>> open;

    std::uint32_t seq = 0u;
    gScore[startId] = 0;
    const std::int64_t h0 = static_cast<std::int64_t>(
        std::round(cfg_.heuristic_weight *
                   static_cast<double>(manhattan(startCoord, goalCoord))));
    open.push({AStarKey{h0, 0, seq++}, startId});

    std::uint32_t nodesExpanded = 0u;

    while (!open.empty()) {
        const auto [key, cur] = open.top();
        open.pop();

        // Lazy deletion: skip stale heap entries for already-closed nodes.
        if (closed[cur]) continue;
        closed[cur] = 1u;

        if (cur == goalId) {
            result.success = true;
            NodeId node = goalId;
            while (node != kInvalidNode) {
                result.path.push_back(spec_.decode(node));
                node = cameFrom[node];
            }
            std::ranges::reverse(result.path);
            return result;
        }

        if (++nodesExpanded >= cfg_.max_nodes) break;

        const PatchCoord curCoord = spec_.decode(cur);
        const std::int64_t curG  = gScore[cur];
        assert(curG >= 0 && "A* g_score must be non-negative");

        for (const auto& [dx, dy] : kDirOffset) {
            const int nx = curCoord.x + dx;
            const int ny = curCoord.y + dy;

            if (!spec_.inBounds(nx, ny)) continue;
            if (spec_.stateAt(nx, ny) != TileState::Empty) continue;

            const NodeId nid = spec_.encode(nx, ny);
            if (closed[nid]) continue;

            const std::int64_t tentativeG = curG + 1;
            if (tentativeG < gScore[nid]) {
                gScore[nid]   = tentativeG;
                cameFrom[nid] = cur;

                const PatchCoord nCoord{nx, ny};
                const std::int64_t h = static_cast<std::int64_t>(
                    std::round(cfg_.heuristic_weight *
                               static_cast<double>(manhattan(nCoord, goalCoord))));
                const std::int64_t f = tentativeG + h;
                open.push({AStarKey{f, -tentativeG, seq++}, nid});
            }
        }
    }

    return result; // unreachable goal or max_nodes exceeded
}

std::vector<RouteResult> AStarRouter::routeAll(
    const std::vector<MergeRequest>& requests) const
{
    std::vector<RouteResult> results;
    results.reserve(requests.size());
    for (const auto& req : requests) {
        results.push_back(route(req));
    }
    return results;
}

} // namespace qfault
