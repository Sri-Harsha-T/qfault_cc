# Active Context — QFault

> **This file is the first thing to read at the start of every Claude session.**
> Update "State of Work" and "Next Action" before ending any session.
> Never delete entries — append or strike through completed items.

_Last updated: 2026-04-25 — Stage 1 implementation complete (93 tests green); only CI (#3) remains_

---

## Current Phase

**Stage 1 of 5: IR + Pass Manager Core** ← IMPLEMENTATION COMPLETE  
Target duration: 3–4 weeks  
Stage gate: ✅ PASSED — `tests/integration/test_ir_two_level.cpp` confirms two-level IR works.

## Active Story

**Issue #3 — 1-D-2: GitHub Actions CI** (only remaining open issue in Stage 1)

## State of Work

- ✅ Project skeleton created (memory-bank/, docs/, .claude/, scripts/)
- ✅ CLAUDE.md, CHANGELOG.md, all memory-bank files initialised
- ✅ ADR template and index created; ADRs 0001–0004 accepted
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
- ⬜ GitHub Actions CI — Issue #3 (requires clang-tidy-18 installed in CI)

**Test count: 93/93 green on gcc-13 and clang-18 with -Werror**

## Recent Decisions (last 5 sessions)

| Date | Decision | ADR |
|------|----------|-----|
| 2026-04-24 | Use ccpm for project management | — |
| 2026-04-24 | GridSynth as default synthesiser; SK as benchmark only | ADR-0002 (draft) |
| 2026-04-24 | Global code distance d for v0.1; variable-d deferred | ADR-0003 (draft) |
| 2026-04-24 | Concepts over virtual inheritance for SynthesisProvider | ADR-0004 (draft) |
| 2026-04-25 | ADR-0001 confirmed: variant<LogicalGate,PatchOp> viable, no structural incompatibility | ADR-0001 accepted |

## Open Blockers

- CI (#3) requires `clang-tidy-18` and `clang-format-18` in the runner image.
  `sudo apt-get install -y clang-tidy-18 clang-format-18` on ubuntu-24.04.

## Next Action — Start Here on Next Session

```
1. Read: CLAUDE.md → this file → CHANGELOG.md "Failed Approaches"
2. Run: ./scripts/quick-test.sh  (cmake via ~/.local/bin/cmake) — expect 93 tests green
3. Remaining Stage 1 work:
   - #3: GitHub Actions CI → .github/workflows/ci.yml
     Uses ubuntu-24.04, installs clang-18/gcc-13/cmake-pip, runs gcc13-debug + clang18-debug presets
     Then Stage 1 is 100% complete.
4. After #3: close Epic #1, start Stage 2 planning.
   Stage 2 = LatticeSurgeryPass (LOGICAL→PHYSICAL lowering), Stim integration.
```

## Failed Approaches — DO NOT RETRY

*(See CHANGELOG.md "Failed Approaches" section for the full table)*

- **gcc-13 `-Wmissing-field-initializers`**: fires when designated init omits fields
  even with correct defaults. Fix: add `= {}` or `= std::nullopt` as default member
  initializers in the struct definition itself.
- **`/*comment*/` as range-for variable name**: `for (const auto& /*x*/ : vec)` is
  invalid C++ (comment is not an identifier). Use a named variable + `(void)x;`.

## Key Architecture (quick reference)

```
include/qfault/
  ir/           GateKind, LogicalGate, LogicalQubit, PatchOp, PatchCoord,
                MeasBasis, PatchOpKind, IRLevel, QFaultIRModule
  passes/       PassBase, PassContext, PassManager, NoOpPass
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
```
