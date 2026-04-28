# ADR-0006: Lattice surgery routing — A* on tile grid + Litinski templates + earliest-available scheduling

**Status:** Accepted
**Date:** 2026-04-26
**Supersedes:** —

---

## Context

Stage 3 must route lattice-surgery operations on a 2D tile grid, choose
layout templates (Litinski 2019 compact / intermediate / fast), and
schedule operations against limited routing channels and factory outputs.
The literature includes optimal SAT-based routing (DASCOT-style SCMR),
ILP/EDPC formulations, and greedy heuristics. NP-hardness of optimal
multi-terminal routing on patch grids is well-established (reduction from
multi-Steiner).

---

## Decision

v0.1 routing uses:

1. **A\* search on the tile grid** with Manhattan-distance heuristic
   (admissible on a 4-connected unit-cost grid).
2. **Litinski 2019 compact / intermediate / fast layout templates** as
   parameterizable presets:
   - Compact: `1.5n + 3` tiles, 9τ per T (11τ with one factory).
   - Intermediate: `2n + 4` tiles, 5τ per T (5.5τ with two factories).
   - Fast: `2n + √(8n) + 1` tiles, 1τ per T (many parallel factories).
3. **Greedy earliest-available scheduling** per Silva et al. 2024
   (arXiv:2405.17688), using the **trivial dependency rule**
   (qubit-disjoint ⇒ commute) as the default for scalability.
4. **Multi-terminal routes** via *modified Mehlhorn* (bidirectional
   Dijkstra between terminal pairs, then Kruskal MST on the complete
   graph), restricted to bus tiles.

Optimality is **not claimed**; the approximation gap is to be measured
against DASCOT and liblsqecc on Stage 3 reference circuits and reported
in the arXiv preprint.

**Sanitizer-clean A\* implementation rules** (from prior research):

- Use `NodeId = uint32_t` indices, not `Node*` pointers (vector
  reallocation invalidates pointers; ASan reports use-after-free).
- Heap key `(f, -g, insertion_seq)` for deterministic tie-breaking
  across runs (never compare pointers — ASLR breaks goldens).
- Dense `std::vector<uint8_t> closed` rather than
  `std::unordered_set<NodeId>` (cache-friendly, deterministic).
- Signed `int64_t` costs so UBSan catches overflow rather than silently
  wrapping.
- Lazy deletion (test `closed[cur.node]` after pop) — never try to fish
  into `std::priority_queue` for decrease-key.
- Encode boundary type into node identity as `(tile, side)` — routing
  must match X-edge to X-edge and Z-edge to Z-edge.

---

## Alternatives Considered

| Alternative | Why Rejected |
|---|---|
| SAT-optimal SCMR (DASCOT-style) | Solver dependency, scaling cliff at ~50 patches, too research-y for v0.1 |
| ILP / EDPC formulations | Same scaling and dependency concerns |
| Pure greedy without templates | No way to compare against Litinski reference numbers |
| Reinforcement-learning routing | Not reproducible without trained models; unsuitable for OOPSLA artifact evaluation |
| Dijkstra (liblsqecc default) | A\* with Manhattan heuristic gives 2–5× speedup on grid topology — confirmed in liblsqecc's own benchmarks |

---

## Consequences

**Positive:**
- Implementable in 8–12 weeks solo; deterministic; reproducible.
- Litinski templates allow direct reproduction of Litinski 2019 spacetime
  numbers within 10%, the Stage 3 gate.
- Earliest-available scheduling beats SAT-optimal on pre-transpiled
  circuits by 2–12× per Silva 2024, primarily by integrating Clifford
  pushout (O(m) symplectic transpilation, Litinski Appendix D).

**Negative / Trade-offs:**
- Approximation gap vs DASCOT must be measured and disclosed; 10–30%
  spacetime-volume penalty is plausible on adversarial circuits.
- Greedy scheduling can be pessimal on circuits with high factory
  contention; mitigated by template choice (fast template increases
  factory replication).

**Risks:**
- A\* heap memory on >100-patch grids; mitigated by tiered grid
  decomposition.
- Patch routing physics: a length-L route is implemented as **one**
  multi-patch measurement using an L-tile-long ancilla snake — time cost
  is 1τ regardless of L. Decoder runtime grows as O(L log L), which can
  dominate real-time decoding budgets even though logical time stays at
  d cycles. Document explicitly in the Stage 3 exit report.
- Litinski Fig. 13a caption says "2.5n+4" while body says "2n+4" — use
  the body-text value and document the discrepancy in Stage 3 tests.

---

## Implementation Notes for AI Sessions

When loading the memory bank: routing is A\* + Litinski templates + greedy
scheduling. Do NOT introduce a SAT solver dependency. The `RoutingPass`
interface is at `include/qfault/passes/routing/RoutingPass.hpp` (Stage 3
deliverable). When merging patches, always pair X-boundary with X-boundary
or Z-boundary with Z-boundary; mixing is a logic error and must trip a
runtime assertion (see `.claude/rules/qec.md`).

For the BV-10 d=5 reference: 11 logical qubits × (2d²−1) = 539 base
physical qubits, rising to 600–800 with routing patches; ~20–25 syndrome
rounds; ~1000–1250 detectors; 10 observables. Use
`stim::generate_surface_code_circuit(params)` with all noise probabilities
set to 0 for the noiseless reference patch.

---

## References

- Litinski 2019, *A Game of Surface Codes* (arXiv:1808.02892)
- Silva et al. 2024, *Multi-Qubit Lattice Surgery Scheduling*
  (arXiv:2405.17688)
- Beverland 2022 Appendix C (template parameters)
- liblsqecc (https://github.com/latticesurgery-com/liblsqecc) — reference
  implementation, GPL-3.0 (do NOT vendor)
- DASCOT (OOPSLA 2025, arXiv:2311.18042) — comparison baseline
- LaSsynth (ISCA 2024, arXiv:2404.18369) — comparison baseline
- ADR-0007 (MSD factory selection — consumes routing output)
