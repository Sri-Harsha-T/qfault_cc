---
paths:
  - "src/qfault/passes/routing/**"
  - "include/qfault/passes/routing/**"
  - "tests/unit/test_AStar*.cpp"
  - "tests/unit/test_LatticeSurgery*.cpp"
  - "tests/integration/test_routing*.cpp"
---

# Routing Domain Rules for QFault Stage 3

These rules apply to lattice surgery routing code. Read in conjunction with
`.claude/rules/qec.md`. Read `docs/glossary.md` if any term below is unfamiliar.

## A* implementation rules (ADR-0006)

**NodeId is `uint32_t`, never `Node*`.** Pointer comparison breaks reproducibility
under ASLR — golden files differ across runs. Pointer-based priority queues
explode under ASan when underlying vectors reallocate. Logged as Failed Approach
2026-04-26.

```cpp
using NodeId = std::uint32_t;
constexpr NodeId kInvalidNode = std::numeric_limits<NodeId>::max();
```

**Heap key is `(f_score, -g_score, insertion_seq)`.** This guarantees:
- Lowest f_score wins (A* correctness)
- On tie: deeper search wins (deeper = higher g, negated for min-heap)
- On further tie: earlier insertion wins (insertion_seq is monotonic across runs)

```cpp
struct AStarKey {
    std::int64_t f_score;
    std::int64_t neg_g_score;     // -g, so larger g sorts as smaller key
    std::uint32_t insertion_seq;
    auto operator<=>(const AStarKey&) const = default;
};
```

**Costs are signed `int64_t`.** UBSAN catches overflow. Unsigned silently wraps.
Even though edge costs are non-negative, total f_score can overflow at scale —
use signed and assert non-negativity.

**Closed list is `std::vector<uint8_t>` indexed by NodeId.** Cache-friendly,
deterministic, sanitizer-clean. Do NOT use `std::unordered_set<NodeId>` — its
iteration order is implementation-defined and breaks goldens.

**Lazy deletion, never decrease-key.** `std::priority_queue` has no decrease-key
operation. The pattern is: when you find a better path to a node already in the
open set, push a NEW entry with the better key; on pop, test `if (closed[cur.node]) continue;`.
Memory bound: O(E) heap entries (one per relaxation), not O(V).

**Heuristic: Manhattan distance.** `|Δx| + |Δy|` on a 4-connected unit-cost grid
is admissible and consistent. Use **weighted A*** (`f = g + ε·h`, ε ∈ [1.0, 1.2])
to trade optimality for speed at scale; document ε in ADR-0006 and the test
goldens are pinned to a specific ε.

**Boundary-aware routing.** A patch's boundary side determines what merges legally.
Encode boundary type into NodeId composition:

```cpp
struct TileSide { Tile tile; Side side; }; // X-rough or Z-rough
NodeId encode(TileSide ts);
TileSide decode(NodeId id);
```

Routing must match X-edge to X-edge or Z-edge to Z-edge — mixing is a logic error
that triggers a runtime assertion in `MergeOp::validate()`.

## Multi-terminal routes (Pauli-product measurements with ≥3 patches)

Follow Silva et al. 2024 §3:

1. **Pairwise A***: for each pair of terminals, run A* to get distance
2. **Build complete graph** on terminals with these distances as edge weights
3. **Kruskal MST** on the complete graph
4. **Materialize the tree** by re-running A* between adjacent terminals in the MST

Do NOT attempt direct multi-source A* in v0.1 — it has subtle correctness issues
under our heuristic and is a Stage 6 optimization.

## Litinski layout templates (ADR-0019, ADR-0020)

Three layouts, parameterized by **n** (logical qubit count):

| Layout | Tile count | Cycles per T | Factories needed | Source |
|--------|------------|--------------|------------------|--------|
| Compact | `1.5n + 3` | 9τ (limit 11τ) | 1 (15-to-1 unit) | GoSC §5.1 |
| Intermediate | `2n + 4` | 5τ (5.5τ matched) | 2 | GoSC §5.2 |
| Fast | `2n + √(8n) + 1` | 1τ | many parallel | GoSC §5.3 |

**Caveat:** Litinski Fig. 13a caption says "2.5n+4" but body text says "2n+4".
Use the body text. Document this in ADR-0019.

**Fast block formula caveat:** `2n + √(8n) + 1` is exact only when n/2 is a
perfect square; otherwise round side length UP and shorten the last column.
Implement as:

```cpp
constexpr std::size_t fastBlockTileCount(std::size_t n) {
    // Side length = ceil(sqrt(2n))
    const std::size_t side = static_cast<std::size_t>(std::ceil(std::sqrt(2.0 * n)));
    return 2 * n + side + 1;
}
```

**Geometric rules:**
- One τ ≈ d code cycles (one syndrome-extraction round)
- One tile = d² physical data qubits (≈ 2d² with measure ancillas)
- Adjacent data patches require **≥ 1 empty tile gap** for boundary redefinition
- Routing patches are ancilla snakes occupying L tiles for length-L route
- **Long-range CNOT is 1τ regardless of L** — only spatial footprint scales linearly

## CNOT recipe (ADR-0020)

Local CNOT (3-patch L-shape, control + ancilla + target):

```
Cycle 1 (1τ):  MZZ(C, A) → outcome m1, then SPLIT
Cycle 2 (1τ):  MXX(A, T) → outcome m2
End of cycle 2: MEASURE A in X → outcome mA

Total: 2d code cycles (= 2τ), not d.
Pauli-frame correction: (X_T)^a · (Z_C)^b
  where a = m2 ⊕ mA, b = m1 (or similar — derive per Horsman §4.1)
```

**The IR must produce 4 PatchOp instructions plus a classical PauliFrameUpdate
annotation.** Do not emit 4 separate logical CNOT operations — that's the wrong
abstraction level for the LOGICAL → PHYSICAL transition.

**The Pauli-frame correction is tracked classically and absorbed into upcoming
π/8 rotations.** Never execute it as a physical X/Z gate. See GoSC Fig. 4 for
commutation rules.

## Silva 2024 EAF scheduler

The scheduler runs after the A* router has placed all logical qubits but before
output emission. Algorithm 1 (dependency graph build) uses the **trivial
commutation rule** by default: two operations commute iff they act on disjoint
qubit sets. This is conservative but **scales to ~23M-gate circuits**.

Algorithm 2 (per-step forest packing) — for each candidate set of mutually
commuting operations:

1. Each operation needs a Steiner tree connecting its terminal patches via bus tiles
2. Trees from different operations must be edge-disjoint (no bus tile shared)
3. Solve as a forest-packing problem: greedy by terminal count, ties broken by
   shortest pairwise A* sum

**Throughput target:** tens of thousands of operations scheduled per second.
If the scheduler drops below 10k ops/sec, profile the A* inner loop first
(it's almost always the bottleneck).

## Stim oracle integration (ADR-0021)

When emitting Stim circuits for the oracle:

- The PHYSICAL IR maps to Stim **stabilizer circuit** instructions:
  `H`, `CX`, `CZ`, `M`, `MX`, `MZ`, `R`, `RX`, `DETECTOR`, `OBSERVABLE_INCLUDE`,
  `REPEAT n { ... }`, `TICK`, `SHIFT_COORDS(...)`.
- **Each surface code cycle** is one `REPEAT d { syndrome_extraction; TICK; }` block.
- **Generate noiseless reference patches** via `stim::generate_surface_code_circuit(params)`
  with all four noise probabilities = 0.
- Pin SIMD width: `-DSIMD_WIDTH=64` in CMake (otherwise AVX/SSE seeds give different shots).

**Detector definitions:**
- First round: detector references reset state + first-round measurement only
- Bulk rounds: detector references current AND previous-round measurements of
  the same stabilizer
- Defining a detector on a non-deterministic measurement raises
  `InvalidGaugeException` from `detector_error_model()` — verify each detector
  is deterministic in noiseless simulation before committing to a circuit.

**Equivalence checking:**
- Primary: `stim::Circuit::has_flow(stim::Flow)` — define logical-level flows
  (e.g., for BV-10: 10 `Z_i → Z_i` flows + X-flows mediated by the secret string)
- Backstop: `circuit.without_noise()` + 1024 sweep-bit shots, both circuits
  must produce zero detector flips and matching observable parities

**Forbidden Stim patterns:**
- Empty `REPEAT(0) { ... }` — illegal, parser rejects
- `OBSERVABLE_INCLUDE` referencing detector indices instead of measurement records
- `detector_error_model()` on noiseless circuits — returns empty DEM, fails CI assertions
- Mixing SIMD widths across translation units — ODR violation, segfaults at runtime

## BV-10 reference circuit (Stage 3 gate)

For the Stage 3 correctness gate at d=5:

- 11 logical qubits (10 inputs + 1 ancilla)
- Per-patch physical: `2d² − 1 = 49` qubits (25 data + 24 measure)
- Base physical: `11 × 49 = 539` qubits
- With routing patches: **600–800 physical** (depends on CNOT scheduling)
- Syndrome rounds: ~20–25 (≈ 4 logical layers × d=5)
- Detectors: ~1000–1250
- Observables: 10 (one per input qubit)

Stim equivalence check:
1. Compile BV-10 logical circuit to PHYSICAL IR via `LatticeSurgeryPass`
2. Emit Stim circuit via `StimBackend`
3. Check `circuit.has_flow(...)` for all 10 logical Z observables
4. Check `circuit.without_noise().compute_stats()` shows all detectors deterministic

## Reproducibility for routing goldens

Routing decisions are deterministic given:
1. Input circuit (SHA-256 of canonical text form)
2. Layout choice (compact / intermediate / fast)
3. Code distance d
4. A* tie-break ε
5. Random seed (set to 0 for goldens)

Commit golden routes as JSON 3D-array files (Silva 2024 / liblsqecc convention):
each slice is a 2D snapshot at one τ. Diff goldens with `git diff --no-textconv`
to see precisely what changed when a routing decision flips.

When updating goldens (intentionally), set `QFAULT_UPDATE_GOLDENS=1` in the env;
the test harness will rewrite goldens and pass. CI must NEVER set this variable.

## What NOT to do

- Do NOT vendor `liblsqecc` source code — it is GPL-3.0, incompatible with
  QFault's Apache 2.0 license (ADR-0015). Compare against it via subprocess
  in `bench/`, do not link.
- Do NOT use Dijkstra as the default router. liblsqecc's default is Dijkstra,
  which is wrong for grid topology — A* with Manhattan dominates. (We are
  intentionally not following liblsqecc's default here.)
- Do NOT introduce a SAT solver dependency for routing. SAT-optimal SCMR
  (DASCOT) is a Stage 6+ comparison baseline, not a v0.1 dependency.
- Do NOT introduce reinforcement-learning routing — non-reproducible without
  trained model artifacts; unsuitable for ACM artifact evaluation.
- Do NOT use the deprecated `liblsqecc` DAG pipeline mode pattern (it has known
  scaling cliffs); follow its **streaming pipeline pattern** (one slice at a
  time, never holding the full circuit) for the inspiration only.