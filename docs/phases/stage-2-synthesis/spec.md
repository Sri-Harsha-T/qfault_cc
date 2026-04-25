# Stage 2 Spec: Synthesis Pass (T-Gate)

**Stage:** 2 of 5
**Target duration:** 3–4 weeks
**GitHub Milestone:** "Stage 2: Synthesis Pass"
**Depends on:** Stage 1 complete (✅)

---

## Goal

Implement the `TGateSynthesisPass` that replaces each T/Tdg gate in a
LOGICAL-level `QFaultIRModule` with a Clifford+T sequence that approximates
R_z(π/4) to within synthesis precision ε. The pass is parameterised by a
`SynthesisProvider` C++20 Concept so that GridSynth and Solovay-Kitaev can
be swapped transparently.

**Stage Gate (hard stop before Stage 3):**
> Running `TGateSynthesisPass<GridSynthProvider>` on a 1000-gate Clifford+T
> circuit produces T-counts within 1% of direct GridSynth output, and the
> total pass runtime overhead (vs calling GridSynth directly for each gate)
> is ≤5%.

---

## ADRs to Finalise Before Coding

| ADR | Topic | Current Status |
|-----|-------|---------------|
| ADR-0002 | `SynthesisProvider` via C++20 Concept | Draft — finalise |
| ADR-0003 | Global code distance `d` for v0.1 | Draft — finalise |
| ADR-0004 | GridSynth as default synthesiser | Draft — finalise |

All three must be moved to **Accepted** before any Stage 2 code is written.

---

## Epics and Stories

### Epic 2-A: SynthesisProvider Concept + Providers

**Goal:** Define the abstract interface for synthesis providers and implement
GridSynth (primary) and Solovay-Kitaev (benchmark) as concrete providers.

| Story | Acceptance Criteria | Est. |
|-------|---------------------|------|
| 2-A-1: `SynthesisProvider` C++20 Concept | Concept requires `synthesise(angle, eps) -> std::vector<GateKind>` and `name() -> std::string_view`; `static_assert` in test file | 1d |
| 2-A-2: `GridSynthProvider` | Calls `gridsynth` binary as a subprocess; parses gate sequence from stdout; gracefully skips (returns empty sequence) if binary not found | 2d |
| 2-A-3: `SKProvider` (Solovay-Kitaev) | Pure C++ implementation; depth-3 recursion; produces a valid Clifford+T approximation for any R_z(θ) with ε ≤ 1e-3 | 3d |
| 2-A-4: Unit tests for both providers | `static_assert(SynthesisProvider<GridSynthProvider>)`; `static_assert(SynthesisProvider<SKProvider>)`; round-trip test: synthesise → verify gate sequence is a valid Clifford+T word | 1d |

### Epic 2-B: TGateSynthesisPass

**Goal:** A concrete compiler pass that applies a `SynthesisProvider` to every
T/Tdg gate in the instruction list.

| Story | Acceptance Criteria | Est. |
|-------|---------------------|------|
| 2-B-1: `TGateSynthesisPass<Provider>` template | Satisfies `PassConcept`; `requiredLevel()` returns `LOGICAL`; iterates instructions, replaces T/Tdg with provider output; preserves all other gates unchanged | 2d |
| 2-B-2: Integration test — T-gate replacement | Parse a 10-gate circuit with 3 T-gates → run `TGateSynthesisPass<SKProvider>` → assert: (a) no T/Tdg remain in output, (b) instruction count increased, (c) all remaining gates are Clifford | 1d |
| 2-B-3: T-count validation vs GridSynth reference | For a set of 5 standard angles (π/4, π/8, π/16, π/32, 3π/8), compare T-count from `GridSynthProvider` against published GridSynth benchmark table; assert within 1% | 2d |

### Epic 2-C: CMake + CI Integration

**Goal:** GridSynth is an optional dependency; CI must still be green without it.

| Story | Acceptance Criteria | Est. |
|-------|---------------------|------|
| 2-C-1: CMake finds GridSynth binary | `find_program(GRIDSYNTH_BINARY gridsynth)`; sets `QFAULT_HAS_GRIDSYNTH` cache variable; passes `GRIDSYNTH_BINARY` path to tests via compile definition | 1d |
| 2-C-2: GridSynth tests skip gracefully | Tests that need GridSynth use `GTEST_SKIP()` when `QFAULT_HAS_GRIDSYNTH` is OFF; CI remains green without GridSynth installed | 0.5d |
| 2-C-3: Stage gate benchmark | `scripts/bench-synthesis.sh` runs `TGateSynthesisPass<GridSynthProvider>` on a 1000-gate circuit and reports overhead; target ≤5% | 1d |

---

## Stage 2 Definition of Done

- [ ] ADRs 0002, 0003, 0004 all moved to Accepted
- [ ] All story ACs pass (`ctest -j` green on clang-18 AND gcc-13 in debug and release)
- [ ] ASAN + UBSAN clean on all test targets
- [ ] `clang-tidy` clean on all new `src/` files
- [ ] T-count validation: GridSynth output within 1% of reference tables
- [ ] Stage gate benchmark: overhead ≤5% on 1000-gate circuit
- [ ] Integration test: no T/Tdg gates remain after `TGateSynthesisPass`
- [ ] `memory-bank/activeContext.md` updated with Stage 3 "Next Action"
- [ ] `memory-bank/progress.md` Stage 2 row marked complete with date
- [ ] `CHANGELOG.md` updated
- [ ] GitHub Milestone "Stage 2" closed; all issues Done

---

## Source Layout (Stage 2 additions)

```
include/qfault/passes/synthesis/
  SynthesisProvider.hpp    # C++20 Concept definition
  GridSynthProvider.hpp    # GridSynth binary wrapper
  SKProvider.hpp           # Solovay-Kitaev pure C++ impl
  TGateSynthesisPass.hpp   # Template pass over SynthesisProvider

src/qfault/passes/synthesis/
  GridSynthProvider.cpp    # Subprocess logic + stdout parsing
  SKProvider.cpp           # SK recursion implementation

tests/unit/
  test_GridSynthProvider.cpp
  test_SKProvider.cpp
  test_TGateSynthesisPass.cpp

tests/integration/
  test_synthesis_roundtrip.cpp  # parse → synthesise → no T gates remain

scripts/
  bench-synthesis.sh       # overhead benchmark
```

---

## Prompt Plan for Claude Code Sessions

```
Session ADR: "Finalise ADRs 0002, 0003, 0004 for Stage 2. Review current Draft
  status, confirm decisions hold, move to Accepted. Record any new constraints
  discovered in Stage 1."

Session 2-A-1+2: "Define SynthesisProvider C++20 Concept in
  include/qfault/passes/synthesis/SynthesisProvider.hpp.
  Implement GridSynthProvider: call 'gridsynth' binary as subprocess,
  parse the resulting gate sequence, return std::vector<GateKind>.
  Gracefully skip if binary is absent. Unit test."

Session 2-A-3+4: "Implement SKProvider (Solovay-Kitaev) in pure C++.
  Target: ε ≤ 1e-3 for arbitrary R_z(θ). Unit test covers:
  static_assert SynthesisProvider<SKProvider>, round-trip approximation check."

Session 2-B-1+2: "Implement TGateSynthesisPass<Provider> satisfying PassConcept.
  Integration test: 10-gate circuit with 3 T-gates → no T/Tdg remain after pass."

Session 2-B-3: "T-count validation. Compare GridSynthProvider output against
  published GridSynth benchmark table for 5 standard angles. Assert within 1%."

Session 2-C: "CMake: find_program(gridsynth), QFAULT_HAS_GRIDSYNTH cache var,
  GTEST_SKIP() in GridSynth tests when absent.
  Write scripts/bench-synthesis.sh. Confirm CI green without GridSynth."

Session GATE: "Run Stage 2 gate check. All tests green on clang-18 + gcc-13.
  ASAN clean. Overhead benchmark ≤5%. Update ADRs, activeContext, progress.md,
  CHANGELOG. Close Milestone Stage 2."
```
