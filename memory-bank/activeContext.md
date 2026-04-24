# Active Context — QFault

> **This file is the first thing to read at the start of every Claude session.**
> Update "State of Work" and "Next Action" before ending any session.
> Never delete entries — append or strike through completed items.

_Last updated: 2026-04-25 — GitHub Issues created; starting 1-D-1_

---

## Current Phase

**Stage 1 of 5: IR + Pass Manager Core**  
Target duration: 3–4 weeks  
Stage gate: IR can cleanly represent both logical Clifford+T gates AND surface
code patch operations in the same data structure without a lossy conversion.

## Active Story

**Issue #8 — 1-A-3: QFaultIRModule** (next up; depends on #6 ✅ and #7 ✅)

Also open: #3 (CI), #11 (PassBase), #12 (PassContext) — can start in parallel with #8.

## State of Work

- ✅ Project skeleton created (memory-bank/, docs/, .claude/, scripts/)
- ✅ CLAUDE.md, CHANGELOG.md, all memory-bank files initialised
- ✅ ADR template and index created; ADRs 0001–0004 accepted
- ✅ ccpm skills placed in .claude/skills/ccpm
- ✅ CLAUDE.md improved (directory layout, active stage, QEC invariants, ADR table)
- ✅ Stage 1 phase directory renamed: docs/phases/stage-1-ir-pass-manager/
- ✅ GitHub Issues created: Epic #1 + 17 task issues (#2–#18), Milestone #6
- ✅ Local epic files: .claude/epics/stage-1-ir-pass-manager/ with github-mapping.md
- ✅ CMakeLists.txt + CMakePresets.json — Issue #2 (b262a63)
- ✅ .clang-tidy, .clang-format, compiler presets (clang18/gcc13) — Issue #4 (2503817)
- ✅ scripts/quick-test.sh — Issue #5 (2503817)
- ✅ LogicalQubit, GateKind, LogicalGate — Issue #6 (2503817)
- ✅ PatchCoord, MeasBasis, PatchOpKind, PatchOp — Issue #7 (2503817)
- ⬜ QFaultIRModule (IRLevel, variant, assertLevel) — Issue #8 (unblocked)
- ⬜ PassBase + PassContext — Issues #11, #12 (unblocked, parallel with #8)
- ⬜ GitHub Actions CI — Issue #3 (unblocked)

## Recent Decisions (last 5 sessions)

| Date | Decision | ADR |
|------|----------|-----|
| 2026-04-24 | Use ccpm for project management | — |
| 2026-04-24 | GridSynth as default synthesiser; SK as benchmark only | ADR-0002 (draft) |
| 2026-04-24 | Global code distance d for v0.1; variable-d deferred | ADR-0003 (draft) |
| 2026-04-24 | Concepts over virtual inheritance for SynthesisProvider | ADR-0004 (draft) |

## Open Blockers

- None yet (pre-implementation)

## Next Action — Start Here on Next Session

```
1. Read: CLAUDE.md → this file → CHANGELOG.md "Failed Approaches"
2. Run: ./scripts/quick-test.sh  (cmake via ~/.local/bin/cmake)
3. Next issues (all unblocked):
   - #8: QFaultIRModule (IRLevel enum, variant<LogicalGate,PatchOp>, assertLevel)
         Read: docs/adr/0001-ir-representation.md + .claude/epics/.../8.md
   - #11: PassBase abstract class (can parallel with #8)
   - #12: PassContext (code distance d, diagnostics, timer) (can parallel with #8)
   - #3: GitHub Actions CI (.github/workflows/ci.yml) — requires clang-tidy-18 installed
   Suggested order: do #8 first (critical path), then #11+#12 in same session.
4. Dependency graph: .claude/epics/stage-1-ir-pass-manager/github-mapping.md
```

## Failed Approaches — DO NOT RETRY

*(See CHANGELOG.md "Failed Approaches" section for the full table)*

- **No entries yet** — project just initialised

## Files Touched This Session

```
CLAUDE.md
CHANGELOG.md
memory-bank/projectbrief.md
memory-bank/productContext.md
memory-bank/systemPatterns.md
memory-bank/techContext.md
memory-bank/activeContext.md   ← this file
memory-bank/progress.md
docs/architecture.md
docs/glossary.md
docs/adr/README.md
docs/adr/template.md
docs/adr/0001-ir-representation.md
docs/adr/0002-synthesis-provider.md
docs/adr/0003-code-distance-scope.md
docs/adr/0004-concepts-over-virtual.md
docs/phases/stage-1-ir-pass-manager/spec.md
docs/phases/stage-1-ir-pass-manager/todo.md
ROADMAP.md
.claude/commands/resume.md
.claude/commands/adr.md
.claude/commands/phase-kickoff.md
.claude/commands/phase-exit.md
.claude/commands/log.md
.claude/rules/cpp.md
.claude/rules/qec.md
.claude/settings.json
```
