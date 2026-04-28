# Stage 3 — Kickoff

> First-day playbook. Run when starting Stage 3 work. Assumes Stage 2.5 is ✅.

## Pre-flight checklist

- [ ] Stage 2 is ✅ closed (211/211 tests passing); exit-report committed
- [ ] **Stage 2.5 is ✅ closed** (Stim + QCEC integration in place; Dockerfile;
      flake.nix; goldens for Stage 2 numbers)
- [ ] All ADRs 0001–0021 are committed; ADR-0006, 0019, 0020, 0021 are Accepted
      (these govern Stage 3 directly)
- [ ] `bench/circuits/qasmbench` submodule pinned at exact commit
- [ ] CI is green on `main`

If any pre-flight item is unchecked, **STOP and resolve before kicking off Stage 3.**

## First session — operating procedure

1. Read in this order (the reviewer's discipline):
   - `CLAUDE.md` (full)
   - `memory-bank/activeContext.md`
   - `memory-bank/progress.md`
   - `CHANGELOG.md` "Failed Approaches" (8 entries)
   - `docs/adr/{0006,0019,0020,0021}.md` (the four Stage 3 governing ADRs)
   - `.claude/rules/{cpp,qec,routing}.md`
   - `docs/phases/stage-3-lattice-surgery/spec.md`
   - `docs/phases/stage-3-lattice-surgery/todo.md`
   - `docs/phases/stage-3-lattice-surgery/prompt_plan.md`

2. Confirm understanding: write a one-paragraph summary in chat of "what is the
   Stage 3 gate, in your own words, and what could falsify it?" — if you can't,
   re-read.

3. Run `/pm:prd-parse docs/phases/stage-3-lattice-surgery/spec.md` to generate
   the epic and Story-level GitHub Issues (one issue per S3.x story).

4. Run `/pm:epic-decompose <epic-id>` to generate task-level Issues per the
   `todo.md` task list.

5. Pick the first task with `/pm:next` — should be **#3.1.1** (create the
   `AStar.hpp` header with sanitizer-clean type aliases).

6. Open a worktree branch (`/pm:issue-start 3.1.1`) and start.

## Critical reminders

### Three retractions from the Stage 2 retrospective

These are corrections to prior assumptions; do **not** re-introduce them:

1. **Litinski 2019 Section 5 contains NO BV-10/QFT/adder reference numbers.**
   It is parametric (n=100, T=10⁸, d=13). Do not target per-circuit Litinski
   numbers; target the parametric tile-count formulas.

2. **Beverland 2022 Appendix C contains only 15-to-1 protocols.**
   116-to-12 is Bravyi-Haah 2012; 225-to-1 is Litinski 2019 derivative. Do
   not attribute either to Beverland.

3. **liblsqecc's default router is Dijkstra, not A*.**
   We deviate intentionally. Do not "follow upstream defaults" mindlessly.

### Sanitizer-clean A* contract (Failed Approach 2026-04-26)

If you find yourself writing `std::priority_queue<Node*>` or comparing pointers,
**STOP**. Re-read `.claude/rules/routing.md` §"A* implementation rules". The
correct pattern is `NodeId = uint32_t`, `(f, -g, insertion_seq)` heap key,
signed `int64_t` costs, dense `vector<uint8_t>` closed list, lazy deletion.

### Stim integration discipline (ADR-0021)

- Stim version pinned to **v1.15.0**; do not bump.
- Library target is **`libstim`** (not `stim`).
- SIMD width pinned to **64** for goldens.
- Stim is private to `verify/` and `tests/`; never in public headers.
- Primary equivalence: **`circuit.has_flow(Flow)`**.
- Backstop: **`circuit.without_noise()` + 1024 sweep-bit shots**.

### Forbidden Stim patterns (CI will fail)

- empty `REPEAT(0) { ... }`
- first-round detector referencing prior-round measurements
- mixing SIMD widths across translation units
- `OBSERVABLE_INCLUDE` referencing detector indices
- `circuit.detector_error_model()` on noiseless circuits

## What good looks like at the end of week 1

- A* router skeleton in `include/qfault/passes/routing/AStar.hpp` and
  `src/qfault/passes/routing/AStar.cpp`
- 3–5 unit tests in `tests/unit/test_AStar.cpp`, all green on gcc-13 + clang-18
- ASAN+UBSAN clean on the new tests
- One micro-benchmark in `bench/scripts/bench-astar.sh` that confirms ≥ 10k
  routes/sec on a 100-patch synthetic grid
- Story #3.1.1 closed; #3.1.2 in progress
- `memory-bank/activeContext.md` updated with current task and any gotchas
  encountered

## What good looks like at the end of week 4

- Stories S3.1, S3.2, S3.3 all ✅
- BV-10 d=5 layout produces a valid `PatchSpec` with the expected 24 tiles
  (intermediate layout, n=10)
- `LatticeSurgeryPass` emits the 4-PatchOp + PauliFrameUpdate pattern for
  CNOT correctly
- All Stage 3 unit tests passing; integration tests stubbed but green
- One CHANGELOG entry per significant Failed Approach encountered

## What good looks like at gate close (week 8–12)

- `tests/integration/test_stage_3_gate.cpp` green on CI
- `bench/golden/bv10-d5/` committed with `circuit.stim`, `circuit.dem`,
  `reference_sample.dets`, `stim_version.txt`
- Stim oracle confirms equivalence on all 10 BV-10 logical observables
- Tile counts reproduce Litinski formulas within 10% (compact, intermediate, fast)
- Exit report written; CHANGELOG entry; v0.1.0-stage3 tag pushed
- `memory-bank/progress.md` updated; `activeContext.md` "Next Action" set to Stage 4

## When to escalate to the user

- Any Stim API change or upstream regression that requires a tag bump
- Any case where the Stim oracle and MQT QCEC disagree on equivalence
  (this would be a research-grade finding, not a bug)
- Any A* benchmark that drops below 5k routes/sec on a 100-patch grid
- Any case where a Failed Approach from another stage's CHANGELOG appears
  to be re-emerging (e.g. pointer comparison, signed-overflow, designated-init)
- If after 2 weeks you cannot get a clean BV-10 round-trip, replan the gate