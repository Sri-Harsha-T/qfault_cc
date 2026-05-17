# Active Context — QFault

> **This file is the first thing to read at the start of every Claude session.**
> Update "State of Work" and "Next Action" before ending any session.
> Never delete entries — append or strike through completed items.
> Persistent context for the next AI session. Update before `/clear`.
_Last updated: 2026-05-17 — Stage 2.5 COMPLETE (all 17 issues closed); 144/144 tests green_

---
## Current state (2026-05-17)

- **Stages 1 + 2 + 2.5 COMPLETE.**
- **Stage 2.5:** All 17 issues closed (#29–#46). 144/144 tests pass on gcc13-stim.
- **Exit report written:** `docs/phases/stage-2.5-verification-benchmark-harness/exit-report.md`
- **Next:** Stage 3 — Lattice Surgery Mapper.

## Current Phase

**Stage 3: Lattice Surgery Mapper** — beginning next session.
Previous: Stage 2.5 ✅ COMPLETE — 144 tests green; Stim+QCEC+golden+bench+CI all wired.

## Active Stage

**Stage 3** — Logical CNOT → patch merge/split sequences + A* router.
See `docs/phases/stage-3-lattice-surgery/` for the Stage 3 plan.

## Active Story

Stage 3 has not been started. Next session should:
1. Read `docs/phases/stage-3-lattice-surgery/` (spec, todo, kickoff if they exist)
2. Create Stage 3 GitHub milestone and issues
3. Implement `LatticeSurgeryPass` skeleton + `PatchCoord` routing

## Next Action

**Begin Stage 3: Lattice Surgery Mapper.**
```
1. Read: CLAUDE.md → this file → CHANGELOG.md "Failed Approaches"
2. Run: cmake --preset gcc13-stim && cmake --build build/gcc13-stim -j && ctest --test-dir build/gcc13-stim
   → should be 144/144 green.
3. Begin Stage 3:
   a. Read docs/phases/stage-3-lattice-surgery/ (spec + plan)
   b. Create GitHub milestone "Stage 3: Lattice Surgery"
   c. Implement LatticeSurgeryPass skeleton
4. Stage 3 gate: Stim oracle confirms BV-10 at d=5 produces correct logical output.
```

## State of Work

### Stage 1 (COMPLETE)
- ✅ Project skeleton created (memory-bank/, docs/, .claude/, scripts/)
- ✅ CLAUDE.md, CHANGELOG.md, all memory-bank files initialised
- ✅ ADR template and index created; ADRs 0001–0004 written
- ✅ ccpm skills placed in .claude/skills/ccpm
- ✅ CMakeLists.txt + CMakePresets.json — Issue #2 (b262a63)
- ✅ .clang-tidy, .clang-format, compiler presets (clang18/gcc13) — Issue #4 (2503817)
- ✅ scripts/quick-test.sh — Issue #5 (2503817)
- ✅ LogicalQubit, GateKind, LogicalGate — Issue #6 (2503817)
- ✅ PatchCoord, MeasBasis, PatchOpKind, PatchOp — Issue #7 (2503817)
- ✅ QFaultIRModule (IRLevel, variant, assertLevel, dump) — Issues #8, #9 (30b4142)
- ✅ PassBase + PassContext — Issues #11, #12 (30b4142)
- ✅ Stage gate integration test + ADR-0001 confirmation — Issue #10 (1b05ca4)
- ✅ PassManager add<T>()/run() — Issue #13 (dd025ac)
- ✅ NoOpPass + round-trip integration test — Issue #14 (fc8bdad)
- ✅ PassManager::printStats() — Issue #15 (d081444)
- ✅ QASM 3.0 Lexer (Clifford+T subset) — Issue #16 (6ef5bd4)
- ✅ QASM 3.0 Parser → QFaultIRModule — Issue #17 (6ef5bd4)
- ✅ QASM 3.0 round-trip integration test — Issue #18 (6ef5bd4)
- ✅ GitHub Actions CI matrix + ASAN job — Issue #3 (df0c6ce)
- ✅ UBSAN overflow fix (test_PassContext volatile int → long long)
- ✅ All GitHub issue descriptions populated (was empty)
- ✅ Stage 1 phase docs updated (exit-report, kickoff, todo, spec)

**Test count: 93/93 green on gcc-13 and clang-18 with -Werror and ASAN+UBSAN**

**Stage 2 test count: 118/118 green on gcc-13, clang-18, clang-18-asan**
**Stage 2.5 test count: 144/144 green on gcc13-stim (C++23)**

### Stage 2 (COMPLETE — gate passed 2026-04-28)
- ✅ Stage 2 spec written: `docs/phases/stage-2-synthesis/spec.md`
- ✅ Stage 2 todo written: `docs/phases/stage-2-synthesis/todo.md`
- ✅ Stage 2 kickoff written: `docs/phases/stage-2-synthesis/kickoff.md`
- ✅ ADRs 0002/0003/0004 Accepted
- ✅ Stage 2 Epic + Issues created on GitHub (#19–#28, milestone #7)
- ✅ Stage 2 prompt_plan.md written: docs/phases/stage-2-synthesis/prompt_plan.md
- ✅ #19: SynthesisProvider C++20 Concept (commit 90cc12d)
- ✅ #20: GridSynthProvider subprocess wrapper (commit eb441cb)
- ✅ #21: SKProvider Solovay-Kitaev pure C++ (commit eb441cb)
- ✅ #22: Concept static_asserts for all providers (commit 0e285f2)
- ✅ #23: TGateSynthesisPass<Provider> template pass (commit 0e285f2)
- ✅ #24: Integration test — synthesis round-trip (commit 0e285f2)
- ✅ #25: T-count validation test (GTEST_SKIP when no binary) (commit 2a487b4)
- ✅ #26: CMake GridSynth detection + QFAULT_HAS_GRIDSYNTH (commit eb441cb)
- ✅ #27: GTEST_SKIP() guards verified on all GridSynth tests (commit 2a487b4)
- ✅ #28: scripts/bench-synthesis.sh stage gate benchmark (commit 2a487b4)
- ⬜ Stage gate formal sign-off (requires GridSynth binary installed)

### Stage 2.5 (COMPLETE — 2026-05-17)
- ✅ #29: cmake/dependency_versions.cmake wired
- ✅ #30: Stim v1.15.0 FetchContent + libstim + gcc13-stim preset
- ✅ #31: StimOracle helper (ir_to_stim_text, circuits_clifford_equivalent)
- ✅ #32: Detector-distribution backstop (4 tests)
- ✅ #33: SIMD-width discipline (kStimW=64, static_assert, pragma guards)
- ✅ #34: MQT QCEC v3.5.0 FetchContent
- ✅ #35: QCECBridge (check_equivalence, is_passing, EquivalenceResult)
- ✅ #36: Qubit-threshold dispatch (kQcecStrictThreshold=8)
- ✅ #37: bench/golden/qcec/ (5 circuit pairs + test_qcec_golden.cpp)
- ✅ #38: bench/circuits/qasmbench/ shallow submodule
- ✅ #39: bench/circuits/feynman/ shallow submodule + dotqc_to_qasm.sh
- ✅ #40: bench/scripts/gen_mqtbench.py
- ✅ #41: bench/tier1/run.sh
- ✅ #42: bench/tier2/run.sh
- ✅ #43: Dockerfile multi-stage (builder + runtime)
- ✅ #44: flake.nix
- ✅ #45: bench/Makefile + plot.py + check_regression.py + stage2_baseline.csv
- ✅ #46: CI stim-integration job (.github/workflows/ci.yml)
- ✅ Exit report: docs/phases/stage-2.5-verification-benchmark-harness/exit-report.md
- ✅ Project bumped to C++23 globally

## Recent Decisions (last 5 sessions)

| Date | Decision | ADR |
|------|----------|-----|
| 2026-05-17 | Bumped C++23 globally (gcc-13 + clang-18 both support; needed for std::expected) | — |
| 2026-05-17 | kStimW=64 hardcoded (SIMD_WIDTH CMake var doesn't propagate to consumer TUs) | ADR-0021 |
| 2026-05-17 | Empty Stim circuit anchored with I-gates before tableau comparison | ADR-0021 |
| 2026-05-17 | QFAULT_HAS_STIM propagated to test binary explicitly (not via oracle linkage) | — |
| 2026-05-17 | mqt-core-qasm linked explicitly in qfault_link_qcec() (not transitive from QCEC) | — |

## Open Blockers

- ADR-0011 (phase-polynomial pass) is still Draft; decision deferred until Stage 3 clarity.
- Stage 3 has not been started.
- `flake.lock` not committed (requires `nix flake lock` on Nix-enabled machine).
- GridSynth Haskell binary not in Dockerfile (manual step; documented in Dockerfile).

## Next Action — Start Here on Next Session

```
1. Read: CLAUDE.md → this file → CHANGELOG.md "Failed Approaches"
2. Run: cmake --preset gcc13-stim && cmake --build build/gcc13-stim -j && ctest --test-dir build/gcc13-stim
   → should be 144/144 green. If not, check CHANGELOG Failed Approaches.
3. Begin Stage 3:
   a. Read docs/phases/stage-3-lattice-surgery/ (spec + plan)
   b. Run /pm:epic-decompose to create Stage 3 GitHub issues
   c. Implement LatticeSurgeryPass skeleton
4. Stage 3 gate: circuits_clifford_equivalent() confirms BV-10 at d=5 = correct logical output.
```

## Failed Approaches — DO NOT RETRY

*(See CHANGELOG.md "Failed Approaches" section for the full table — 8+ entries)*

- **gcc-13 `-Wmissing-field-initializers`**: fires when designated init omits fields
  even with correct defaults. Fix: add `= {}` or `= std::nullopt` as default member
  initializers in the struct definition itself.
- **`/*comment*/` as range-for variable name**: `for (const auto& /*x*/ : vec)` is
  invalid C++ (comment is not an identifier). Use a named variable + `(void)x;`.
- **`debug` preset with system compiler**: system default gcc 9.4 cannot compile C++20
  `= default operator==`. Always use `gcc13-debug` or `clang18-debug` named presets.
- **`quick-test.sh` was hardcoded to `build/debug`**: same issue. Fixed to use
  `gcc13-debug` by default (overridable via `QFAULT_PRESET` env var).
- **"no T remain" integration test with SKProvider**: SKProvider returns `{T}` for
  R_z(π/4) (exact answer). Use `CliffordOnlyProvider` mock in the test instead.

## Don't Forget

- **8+ entries in `CHANGELOG.md` "Failed Approaches" — read all before proposing**
  anything in routing, synthesis, A*, or factory cost code.
- Three numerical-target retractions from the retrospective:
  1. Litinski Section 5 has NO BV-10/QFT/adder reference numbers — target the
     parametric tile-count formulas (1.5n+3, 2n+4, 2n+√(8n)+1) instead
  2. Beverland 2022 has only 15-to-1 factories — 116-to-12 (Bravyi-Haah 2012)
     and 225-to-1 (Litinski 2019) are separate sources
  3. liblsqecc's default router is Dijkstra (not A*); QFault deviates intentionally
- **`SKProvider` will be renamed `BFSTableProvider`** in v0.2 (ADR-0013).
- "Validated" not "verified" everywhere (ADR-0009).
- Stim v1.15.0, library target `libstim`, SIMD pinned to 64 (ADR-0021).

## Memory pointers (for /resume)

- `CLAUDE.md` → conventions, all 21 ADRs cross-referenced
- `CHANGELOG.md` → "Failed Approaches" (8+ entries) + stage progress log
- `docs/adr/README.md` → ADR index
- `docs/phases/stage-2.5-verification-benchmark-harness/exit-report.md` → Stage 2.5 exit report
- `docs/phases/stage-3-lattice-surgery/` → Stage 3 plan (ready to kick off)
- `.claude/rules/{cpp,qec,routing}.md` → path-scoped rules
- `.claude/agents/{cpp-pro,reviewer}.md` → specialised subagents

## Key Architecture (quick reference)

```
include/qfault/
  ir/           GateKind, LogicalGate, LogicalQubit, PatchOp, PatchCoord,
                MeasBasis, PatchOpKind, IRLevel, QFaultIRModule
  passes/       PassBase, PassContext, PassManager, NoOpPass
                synthesis/  SynthesisProvider (Concept), GridSynthProvider,
                            SKProvider, TGateSynthesisPass
  frontend/     Lexer, Parser (ParseResult)
  oracle/       StimOracle, QCECBridge (Stage 2.5)
  util/         Overload (std::visit helper)
src/qfault/
  ir/           QFaultIRModule.cpp (dump implementation)
  frontend/     Lexer.cpp, Parser.cpp
  oracle/       StimOracle.cpp, QCECBridge.cpp (Stage 2.5)
tests/
  unit/         test_LogicalGate, test_PatchOp, test_QFaultIRModule,
                test_PassBase, test_PassContext, test_PassManager,
                test_Lexer, test_Parser
  integration/  test_ir_two_level, test_noop_roundtrip, test_qasm_roundtrip,
                test_synthesis_roundtrip, test_stim_oracle, test_stim_detector_dist,
                test_qcec_bridge, test_qcec_golden (Stage 2.5)
bench/
  golden/qcec/  BV-4/6/8, QFT-4, adder-4 circuit pairs + expected verdicts
  golden/       stage2_baseline.csv
  circuits/     qasmbench/ (submodule), feynman/ (submodule)
  scripts/      gen_mqtbench.py, dotqc_to_qasm.sh, plot.py, check_regression.py
  tier1/, tier2/ run.sh harnesses
  Makefile      figures/regression targets
```
