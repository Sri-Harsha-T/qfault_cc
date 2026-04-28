# Stage 3 — Lattice Surgery Mapper

**Estimated duration:** 8–12 weeks
**Difficulty:** ★★★★★ (hardest stage in the project — Stim oracle correctness gate)
**Prerequisites:** Stage 2 ✅, Stage 2.5 ✅ (Stim and QCEC integration must be in place)

---

## Mission

Convert a `LOGICAL` `QFaultIRModule` (Clifford+T circuit, with T-gates already
synthesised in Stage 2) into a `PHYSICAL` `QFaultIRModule` whose `PatchOp`
sequence implements the same logical circuit on a 2D rotated surface code via
**lattice surgery routing** — and **prove correctness against the Stim oracle**.

This is where QFault's "end-to-end pipeline" thesis is either earned or fails.

## Stage gate (falsifiable predicate)

> On the **Bernstein-Vazirani 10-qubit circuit at d=5**, does QFault produce a
> PHYSICAL IR whose Stim emission satisfies `circuit.has_flow(...)` for all
> 10 logical Z observables, AND whose tile-count formulas reproduce within 10%
> of the parametric Litinski 2019 numbers (compact `1.5n+3`, intermediate
> `2n+4`, fast `2n+√(8n)+1`)?

The gate is encoded in `tests/integration/test_stage_3_gate.cpp`. It is **not**:

- "Litinski 2019 BV-10 / QFT / ripple-adder reference numbers" — those don't
  exist in the paper; Section 5 is parametric (logged Failed Approach 2026-04-26)
- "DASCOT-tier optimal routing" — DASCOT is a comparison baseline, not a v0.1
  target

## Acceptance criteria (broken into Stories)

### Story S3.1 — A* router on tile grid

- Given a `PatchSpec` (set of patches with positions and boundary types) and a
  list of merge requests (pairs of `(patch_a, side_a, patch_b, side_b, basis)`),
  produce a list of routing-patch placements that connect each merge request
  via an L-shaped or snake ancilla, **using A*** (Manhattan heuristic).
- **Sanitizer-clean implementation** required:
  - `using NodeId = std::uint32_t;` — never `Node*` (Failed Approach 2026-04-26)
  - Heap key `(f_score, neg_g_score, insertion_seq)` for deterministic tie-break
  - Costs `int64_t` (signed) so UBSAN catches overflow
  - Closed list `std::vector<uint8_t>`, not `unordered_set`
  - Lazy deletion (`if (closed[cur.node]) continue;` after pop)
- Routing must respect boundary types: X-edge ↔ X-edge, Z-edge ↔ Z-edge.
  Encoded into `NodeId` composition (`(tile, side)`) so structural matching
  falls out of A* naturally (ADR-0019).
- ≥ 1 empty tile gap between data patches enforced.
- **Unit tests:** corner-case grids (1×1, 3×3, 10×10, irregular), unreachable
  endpoint case, cycle detection, golden routing for BV-10 d=5 layout.

### Story S3.2 — Litinski layout templates

- `compactLayout(n) → PatchSpec` with `1.5n + 3` tiles, factory in row 0,
  data patches in row 1+
- `intermediateLayout(n) → PatchSpec` with `2n + 4` tiles
  (per body text — Fig. 13a caption "2.5n+4" is a known erratum, document)
- `fastLayout(n) → PatchSpec` with `2n + ⌈√(2n)⌉ + 1` tiles, side rounded up
  when n/2 not a perfect square
- A `LayoutChoice` enum + selection function `chooseLayout(spec) → LayoutChoice`
  based on T-depth and factory-throughput targets
- **Unit tests:** for each layout, assert tile count matches formula for n in
  {1, 2, 4, 8, 10, 16, 100}; assert factory placement matches the published
  diagrams (visual regression test with committed PDF).

### Story S3.3 — Logical CNOT recipe (ADR-0020)

- `LatticeSurgeryPass` emits, for each logical CNOT(C, T):
  - 4 `PatchOp` instructions: MERGE(C, A, Z), SPLIT(C, A), MERGE(A, T, X), MEASURE(A, X)
  - 1 `PauliFrameUpdate` annotation: `(X_T)^a · (Z_C)^b` derived from outcomes
- **2d-cycle cost** (not d) — assert in test
- Long-range CNOT cost is **1τ regardless of length L** — assert in test
- Pauli frame is **classical**, absorbed into upcoming π/8 rotations via
  GoSC Fig. 4 commutation rules — never executed as physical gates
- **Integration test** `test_pauli_frame_absorption.cpp` asserts the absorption

### Story S3.4 — Silva 2024 EAF scheduler

- Build dependency graph using **trivial commutation rule** (qubit-disjoint ⇒
  commute) per Silva Algorithm 1
- For each time step, solve forest-packing: pairwise A* into complete graph,
  Kruskal MST, materialize tree (Silva Algorithm 2)
- Optimization metric: **expected logical-cycle count E(N)**
- Throughput target: ≥ 10k operations / second on the dependency graph
  (profile if below; A* inner loop is usually the bottleneck)

### Story S3.5 — Stim emission backend

- `StimBackend` consumes PHYSICAL IR and emits a `stim::Circuit`
- Each surface code cycle = `REPEAT d { syndrome_extraction; TICK; }` block
- Detector definitions follow the rules:
  - First round: detector references reset state + first-round measurement only
  - Bulk rounds: detector references current AND previous-round measurements
- `OBSERVABLE_INCLUDE` tracks logical observables per ADR-0021
- Forbidden patterns rejected at construction:
  - empty `REPEAT(0)` blocks
  - first-round detectors referencing prior-round measurements
  - mixing SIMD widths across translation units
  - `OBSERVABLE_INCLUDE` referencing detector indices

### Story S3.6 — Stim oracle equivalence check

- `verify/stim_oracle.cpp`: takes two `stim::Circuit`s (compiled and reference),
  returns `Result<EquivalenceVerdict>`
- Primary: `circuit.has_flow(stim::Flow)` for each logical observable
  (10 flows for BV-10; randomized 256-trial check, FP rate 2⁻²⁵⁶)
- Backstop: `circuit.without_noise()` + 1024 sweep-bit shots; both circuits
  must show 0 detector flips and matching observable parities
- Stim version pinned to `v1.15.0` (ADR-0021); SIMD width `-DSIMD_WIDTH=64`

### Story S3.7 — BV-10 reference circuit goldens

- Generate noiseless reference patches via `stim::generate_surface_code_circuit(...)`
  with all four noise probabilities = 0
- Commit four golden files to `bench/golden/bv10-d5/`:
  - `circuit.stim` (text, ~10–50 KB)
  - `circuit.dem` (detector error model, text)
  - `reference_sample.dets` (one shot, sparse `dets` format)
  - `stim_version.txt` (= "v1.15.0\n")
- Update workflow: `QFAULT_UPDATE_GOLDENS=1 ctest -R Golden` rewrites and passes
  (CI must NEVER set this)

### Story S3.8 — Stage 3 gate test

- `tests/integration/test_stage_3_gate.cpp` runs end-to-end:
  1. Parse `bench/circuits/qasmbench/small/bv_n10/bv_n10.qasm`
  2. Run `TGateSynthesisPass<GridSynthProvider>` (or `BFSTableProvider` if no GridSynth)
  3. Run `LatticeSurgeryPass` with `LayoutChoice::Intermediate` at d=5
  4. Emit Stim via `StimBackend`
  5. Compare to `bench/golden/bv10-d5/circuit.stim` exact-match
  6. Run `verify/stim_oracle` — must pass `has_flow` for all 10 observables
  7. Assert physical qubit count ∈ [539, 800]
  8. Assert detector count ∈ [1000, 1250]
  9. Tile count for intermediate layout must equal `2*10 + 4 = 24` exactly

## Out of scope (Stage 4 or later)

- Variable code distance per region (Stage 4, see ADR-0003)
- MSD factory placement and replication (Stage 4)
- Twist defects / LSwT mode for MYY (v0.2+)
- ZX-calculus / phase-polynomial pre-pass (ADR-0011, may slip to Stage 6)
- DASCOT comparison baseline (Stage 5c, paper benchmark only)
- Real-time decoder integration (out of project scope; we emit, we don't decode)

## Risks and mitigations

| Risk | Likelihood | Mitigation |
|------|-----------|------------|
| Stim ABI breaks before Stage 3 ships | Med | Pin tag v1.15.0; FetchContent the source; budget tag-bump as a follow-up ADR |
| A* heap memory on > 100-patch grids | Med | Tiered grid decomposition; profile before optimising |
| Pauli-frame absorption silently doubles T-count | Med | Integration test `test_pauli_frame_absorption` asserts absorption |
| Litinski tile-count formulas have undocumented edge cases | Low | Ceiling-up rule for fast-block side; document the Fig. 13a "2.5n+4" erratum |
| Goldens differ across machines (AVX vs SSE seeds) | High | Pin `-DSIMD_WIDTH=64`; per-platform goldens as escape hatch |
| BV-10 Stim circuit size blows past golden-friendly threshold | Low | If > 1 MB, gitignore and regenerate from seed; use Git LFS as last resort |

## Dependencies

- **External libs:** Stim v1.15.0 (Stage 2.5), MQT QCEC v3.5.0 (Stage 2.5),
  tl::expected v1.1.0 (new in Stage 3 for `Result<T>`)
- **Submodules:** `bench/circuits/qasmbench/` (PNNL/QASMBench at exact commit)
- **ADRs that govern this stage:** ADR-0006, ADR-0019, ADR-0020, ADR-0021,
  with side references to ADR-0001 (IR), ADR-0002 (Concept), ADR-0003 (d),
  ADR-0009 (validation), ADR-0017 (reproducibility)

## Definition of Done (Stage 3)

- [ ] All 8 stories shipped, all unit and integration tests passing
- [ ] BV-10 d=5 golden committed and reproducible across CI
- [ ] Stim equivalence gate green on gcc-13 AND clang-18
- [ ] ASAN+UBSAN clean on all routing code
- [ ] Tile-count formulas reproduce Litinski 2019 within 10%
- [ ] Stage 3 exit-report written at `docs/phases/stage-3-lattice-surgery/exit-report.md`
- [ ] CHANGELOG updated with stage entry and any new Failed Approaches
- [ ] `memory-bank/progress.md` updated to reflect Stage 3 done
- [ ] `memory-bank/activeContext.md` "Next Action" set to Stage 4 kickoff