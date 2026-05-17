# Active Context — QFault

> **This file is the first thing to read at the start of every Claude session.**
> Update "State of Work" and "Next Action" before ending any session.
> Never delete entries — append or strike through completed items.
> Persistent context for the next AI session. Update before `/clear`.
_Last updated: 2026-05-17 — Stage 2.5 Epic A+B complete (Stim+QCEC wired); 8 issues closed; 139 tests green_

---
## Current state (2026-05-17)

- **Stages 1 + 2 COMPLETE.** Stage 2.5 in progress.
- **Stage 2.5 Epic A (Stim) + Epic B (QCEC) COMPLETE.** 139/139 tests pass on gcc13-stim.
- **Project bumped to C++23** (required for std::expected; gcc-13 + clang-18 both support).
- Issues closed today: #29, #30, #31, #32, #33, #34, #35, #36.
- Remaining Stage 2.5: Epic C (benchmark corpus #38-#42), Epic D remainder (#37,#43-#45), CI job (#46).

## Current Phase

**Stage 2.5: Verification & Reproducibility Harness** — Epic A+B complete.
Previous: Stage 2 ✅ COMPLETE — 118 tests green, GridSynth gate passed.

## Active Stage

**Stage 2.5** — benchmark corpus (Epic C) and reproducibility infra (Epic D) remain.
See `docs/phases/stage-2.5-verification-benchmark-harness/`.

## Active Story

Epic C and D have not been started. Next session should work on:
1. `#37`: QCEC golden circuits — commit 5 reference circuits under `bench/golden/qcec/`
2. `#38`: QASMBench git submodule (shallow)
3. `#43`: Dockerfile multi-stage (primary reproducibility deliverable)

## Next Action

**Begin issue #37: create `bench/golden/qcec/` with 5 circuits + expected verdicts.**
Then #38 (QASMBench submodule) and #43 (Dockerfile).

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
**Stage 2.5 (Stim+QCEC) test count: 139/139 green on gcc13-stim (C++23)**

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

## Recent Decisions (last 5 sessions)

| Date | Decision | ADR |
|------|----------|-----|
| 2026-04-25 | TGateSynthesisPass inherits PassBase (virtual at pass level, not gate level) | ADR-0002 impl |
| 2026-04-25 | SKProvider uses depth-7 BFS basic set (~512 entries); not full depth-3 SK recursion | — |
| 2026-04-25 | spec "no T remain" assertion requires CliffordOnlyProvider mock, not SK/GridSynth | — |
| 2026-04-25 | quick-test.sh fixed to use gcc13-debug preset (system gcc 9.4 can't compile C++20) | — |
| 2026-04-25 | ADR-0001 confirmed: variant<LogicalGate,PatchOp> viable | ADR-0001 accepted |

## Open Blockers

- ADR-0011 (phase-polynomial pass) is still Draft; decision deferred until Stage 3 clarity.
- Stage 2.5 Epic C (benchmark corpus) and Epic D (reproducibility infra) not started.
- CI job for Stim/QCEC integration (#46) not added — stim preset exists but no GitHub Actions job.

## Next Action — Start Here on Next Session

```
1. Read: CLAUDE.md → this file → CHANGELOG.md "Failed Approaches"
2. Run: cmake --preset gcc13-stim && cmake --build build/gcc13-stim -j && ctest --test-dir build/gcc13-stim
   → should be 139/139 green. If not, check CHANGELOG Failed Approaches for Stim/QCEC build fixes.
3. Begin Stage 2.5 Epic C/D:
   a. Issue #37: create bench/golden/qcec/ with 5 QASM reference circuits + expected_verdict.txt
   b. Issue #38: git submodule add QASMBench (shallow)
   c. Issue #43: write Dockerfile (Ubuntu 24.04 multi-stage; primary reproducibility gate)
4. When Epic C+D done → /phase-exit for Stage 2.5
5. Then start Stage 3: Lattice Surgery Mapper
```

## Failed Approaches — DO NOT RETRY

*(See CHANGELOG.md "Failed Approaches" section for the full table)*

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

- **8 entries in `CHANGELOG.md` "Failed Approaches" — read all before proposing**
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
- `CHANGELOG.md` → "Failed Approaches" (8 entries) + stage progress log
- `docs/adr/README.md` → ADR index
- `docs/phases/stage-2.5-verification/` → next stage spec
- `docs/phases/stage-3-lattice-surgery/` → full Stage 3 treatment, ready to kick off
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
  util/         Overload (std::visit helper)
src/qfault/
  ir/           QFaultIRModule.cpp (dump implementation)
  frontend/     Lexer.cpp, Parser.cpp
tests/
  unit/         test_LogicalGate, test_PatchOp, test_QFaultIRModule,
                test_PassBase, test_PassContext, test_PassManager,
                test_Lexer, test_Parser
  integration/  test_ir_two_level, test_noop_roundtrip, test_qasm_roundtrip,
                test_synthesis_roundtrip
src/qfault/
  passes/synthesis/  GridSynthProvider.cpp, SKProvider.cpp
docs/phases/
  stage-1-ir-pass-manager/  spec, todo (✅), kickoff, exit-report, prompt_plan
  stage-2-synthesis/        spec, todo (✅ code), kickoff, prompt_plan
```
