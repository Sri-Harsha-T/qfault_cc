# Stage 3 — Task list

> Per-story task breakdown. Use `/pm:epic-decompose` to push these as
> GitHub Issues. Order matches the Stage 3 spec.

## S3.1 — A* router on tile grid

- [ ] **#3.1.1** Create `include/qfault/passes/routing/AStar.hpp` with `NodeId`
      type alias (`uint32_t`), `AStarKey` tuple struct, `AStarConfig` struct
      (heuristic ε, max nodes, deterministic seed)
- [ ] **#3.1.2** Implement `AStar::route(start_node, goal_pred, neighbors_fn)`
      with sanitizer-clean rules from `.claude/rules/routing.md`
- [ ] **#3.1.3** Implement `TileGrid` adjacency representation (8-connected? no,
      4-connected) with `(tile, side)` NodeId composition
- [ ] **#3.1.4** Implement boundary-type check: X-edge ↔ X-edge, Z-edge ↔ Z-edge
      enforced at neighbor generation
- [ ] **#3.1.5** `tests/unit/test_AStar.cpp` — basic 1×1, 3×3, 10×10 grids;
      unreachable goal; symmetric grid (must be deterministic across runs)
- [ ] **#3.1.6** `tests/unit/test_AStar_BoundaryMatch.cpp` — assert mixing X/Z
      boundaries fails at construction
- [ ] **#3.1.7** Profile A* on a 100-patch grid; confirm ≥ 10k routes/sec
- [ ] **#3.1.8** ASAN+UBSAN run on `tests/unit/test_AStar*` — must be clean

## S3.2 — Litinski layout templates

- [ ] **#3.2.1** Create `include/qfault/passes/routing/Layouts.hpp`
- [ ] **#3.2.2** `compactLayout(n)` — return `PatchSpec` with tile count `1.5n+3`
      (use integer arithmetic: `n + n/2 + 3`)
- [ ] **#3.2.3** `intermediateLayout(n)` — tile count `2n+4` (body text;
      document Fig. 13a "2.5n+4" erratum)
- [ ] **#3.2.4** `fastLayout(n)` — tile count `2n + ⌈√(2n)⌉ + 1`
- [ ] **#3.2.5** `chooseLayout(spec)` — selection function based on T-depth
      and factory-throughput
- [ ] **#3.2.6** `tests/unit/test_Layouts.cpp` — assert tile counts for
      n in {1, 2, 4, 8, 10, 16, 100}
- [ ] **#3.2.7** Visual regression test: emit each layout to SVG, compare to
      committed `bench/golden/layouts/{compact,intermediate,fast}_n10.svg`

## S3.3 — Logical CNOT recipe

- [ ] **#3.3.1** Add `PauliFrameUpdate` annotation type to IR
- [ ] **#3.3.2** Implement `LatticeSurgeryPass::emitLocalCNOT(C, T)` →
      4 PatchOps + 1 PauliFrameUpdate, with timeStep on second MERGE = prev_t + 1
- [ ] **#3.3.3** Implement `LatticeSurgeryPass::emitLongRangeCNOT(C, T, length L)` →
      1 multi-patch MERGE (1τ regardless of L)
- [ ] **#3.3.4** Pauli-frame tracker module: `PauliFrameTracker` class that
      accumulates classical Pauli flips and absorbs them into upcoming π/8 rotations
- [ ] **#3.3.5** `tests/integration/test_pauli_frame_absorption.cpp` — assert
      absorption happens, not physical X/Z emission
- [ ] **#3.3.6** `tests/unit/test_LatticeSurgeryPass_CNOT.cpp` — local CNOT
      cost = 2d cycles; long-range = 1τ
- [ ] **#3.3.7** Negative test: `MergeOp::validate()` rejects X↔Z mismatched merge

## S3.4 — Silva 2024 EAF scheduler

- [ ] **#3.4.1** Create `include/qfault/passes/routing/Scheduler.hpp`
- [ ] **#3.4.2** Build dependency graph using trivial commutation (qubit-disjoint)
- [ ] **#3.4.3** Per-step forest-packing: pairwise A* into complete graph,
      Kruskal MST
- [ ] **#3.4.4** Optimization metric `expectedLogicalCycles(schedule)` —
      equivalent to logical depth at fixed d
- [ ] **#3.4.5** `tests/unit/test_Scheduler.cpp` — independence preservation,
      deterministic output, throughput ≥ 10k ops/sec
- [ ] **#3.4.6** Integration test: BV-10 schedule completes in expected logical depth

## S3.5 — Stim emission backend

- [ ] **#3.5.1** Create `verify/StimBackend.hpp` (under verify/, NOT in public
      include path — Stim is a private dependency)
- [ ] **#3.5.2** Each surface-code cycle = `REPEAT d { ... }` block
- [ ] **#3.5.3** Detector-definition rules: first round vs bulk round
- [ ] **#3.5.4** `OBSERVABLE_INCLUDE` per logical Z observable
- [ ] **#3.5.5** Reject empty `REPEAT(0)` at construction
- [ ] **#3.5.6** Reject first-round detector referencing prior rounds
- [ ] **#3.5.7** Reject `OBSERVABLE_INCLUDE` referencing detector indices
- [ ] **#3.5.8** `tests/integration/test_StimBackend.cpp` — small smoke tests
      (1-qubit identity, 1-qubit X, 2-qubit CNOT)

## S3.6 — Stim oracle equivalence check

- [ ] **#3.6.1** Create `verify/StimOracle.hpp`
- [ ] **#3.6.2** `checkHasFlow(circuitA, circuitB, flows)` — primary equivalence
- [ ] **#3.6.3** `checkDetectorMatch(circuitA, circuitB, num_shots=1024)` — backstop
- [ ] **#3.6.4** Both circuits stripped via `circuit.without_noise()` before backstop
- [ ] **#3.6.5** Result type: `tl::expected<EquivalenceVerdict, OracleError>`
- [ ] **#3.6.6** `tests/integration/test_StimOracle_self.cpp` — circuit equals itself
- [ ] **#3.6.7** `tests/integration/test_StimOracle_neg.cpp` — modified circuit fails

## S3.7 — BV-10 reference circuit goldens

- [ ] **#3.7.1** Add `bench/circuits/qasmbench` git submodule at exact commit
- [ ] **#3.7.2** Generate `bench/golden/bv10-d5/circuit.stim` from
      `stim::generate_surface_code_circuit(...)` with all noise = 0
- [ ] **#3.7.3** Generate `bench/golden/bv10-d5/circuit.dem`
- [ ] **#3.7.4** Generate `bench/golden/bv10-d5/reference_sample.dets`
      (sparse `dets` format, NOT `b8`/`r8`)
- [ ] **#3.7.5** Write `bench/golden/bv10-d5/stim_version.txt` ("v1.15.0\n")
- [ ] **#3.7.6** Add `.gitattributes` rule: `bench/golden/** text eol=lf`
- [ ] **#3.7.7** `tests/integration/test_BV10_golden.cpp` — exact-match
      goldens, with `QFAULT_UPDATE_GOLDENS=1` env override

## S3.8 — Stage 3 gate test

- [ ] **#3.8.1** Write `tests/integration/test_stage_3_gate.cpp` covering steps
      1–9 of the spec
- [ ] **#3.8.2** Wire to CI as a required job
- [ ] **#3.8.3** Document in `docs/phases/stage-3-lattice-surgery/exit-report.md`
      template (to be filled at stage close)
- [ ] **#3.8.4** Update `memory-bank/progress.md` to mark Stage 3 ✅ on close
- [ ] **#3.8.5** Bump version tag to `v0.1.0-stage3`

## Cross-cutting

- [ ] **#3.X.1** Update `CHANGELOG.md` with stage entry
- [ ] **#3.X.2** Update `CLAUDE.md` "Active Stage" to Stage 4 on close
- [ ] **#3.X.3** Run full sanitizer matrix across all routing code; log any
      new Failed Approaches
- [ ] **#3.X.4** Add tl::expected v1.1.0 as FetchContent dependency
- [ ] **#3.X.5** Add `cmake/StimSetup.cmake` for the FetchContent + SIMD pinning