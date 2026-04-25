# Active Context — QFault

> **This file is the first thing to read at the start of every Claude session.**
> Update "State of Work" and "Next Action" before ending any session.
> Never delete entries — append or strike through completed items.

_Last updated: 2026-04-25 — Stage 2 GitHub issues #19–#28 created; implementation starting_

---

## Current Phase

**Stage 2 of 5: Synthesis Pass (T-Gate)** ← NEXT TO START
Previous: Stage 1 ✅ COMPLETE — 93 tests green, ADR-0001 Accepted, all 18 issues Done.

## Active Story

**Issue #19** — 2-A-1: SynthesisProvider C++20 Concept (in progress)

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

### Stage 2 (PLANNED, not started)
- ✅ Stage 2 spec written: `docs/phases/stage-2-synthesis/spec.md`
- ✅ Stage 2 todo written: `docs/phases/stage-2-synthesis/todo.md`
- ✅ Stage 2 kickoff written: `docs/phases/stage-2-synthesis/kickoff.md`
- ✅ ADRs 0002/0003/0004 already Accepted (confirmed 2026-04-25)
- ✅ Stage 2 Epic + Issues created on GitHub (#19–#28, milestone #7)
- ✅ Stage 2 prompt_plan.md written: docs/phases/stage-2-synthesis/prompt_plan.md
- 🔄 Stage 2 implementation: #19 in progress

## Recent Decisions (last 5 sessions)

| Date | Decision | ADR |
|------|----------|-----|
| 2026-04-24 | Use ccpm for project management | — |
| 2026-04-24 | GridSynth as default synthesiser; SK as benchmark only | ADR-0004 (draft) |
| 2026-04-24 | Global code distance d for v0.1; variable-d deferred | ADR-0003 (draft) |
| 2026-04-24 | Concepts over virtual inheritance for SynthesisProvider | ADR-0002 (draft) |
| 2026-04-25 | ADR-0001 confirmed: variant<LogicalGate,PatchOp> viable, no structural incompatibility | ADR-0001 accepted |

## Open Blockers

None. Stage 1 is fully closed.

## Next Action — Start Here on Next Session

```
1. Read: CLAUDE.md → this file → CHANGELOG.md "Failed Approaches"
2. Run: cmake --preset gcc13-debug && cmake --build build/gcc13-debug -j && ctest --test-dir build/gcc13-debug
   (The generic 'debug' preset uses system gcc 9.4 which cannot compile C++20 — always use named presets)
3. Resume Stage 2 implementation (issues #19–#28, milestone #7):
   - Next issue: see "Active Story" above
   - Execution order: #19 → #26 (parallel: #20, #21) → #22 → #23 → #24 → #25 → #27 → #28 → gate
   - Prompt plan: docs/phases/stage-2-synthesis/prompt_plan.md
4. After all issues closed: /phase-exit to run stage gate
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

## Key Architecture (quick reference)

```
include/qfault/
  ir/           GateKind, LogicalGate, LogicalQubit, PatchOp, PatchCoord,
                MeasBasis, PatchOpKind, IRLevel, QFaultIRModule
  passes/       PassBase, PassContext, PassManager, NoOpPass
                synthesis/  (Stage 2 — not yet implemented)
  frontend/     Lexer, Parser (ParseResult)
  util/         Overload (std::visit helper)
src/qfault/
  ir/           QFaultIRModule.cpp (dump implementation)
  frontend/     Lexer.cpp, Parser.cpp
tests/
  unit/         test_LogicalGate, test_PatchOp, test_QFaultIRModule,
                test_PassBase, test_PassContext, test_PassManager,
                test_Lexer, test_Parser
  integration/  test_ir_two_level, test_noop_roundtrip, test_qasm_roundtrip
docs/phases/
  stage-1-ir-pass-manager/  spec, todo (✅), kickoff, exit-report, prompt_plan
  stage-2-synthesis/        spec, todo, kickoff  ← NEW
```
