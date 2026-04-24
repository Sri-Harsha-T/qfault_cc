# Active Context — QFault

> **This file is the first thing to read at the start of every Claude session.**
> Update "State of Work" and "Next Action" before ending any session.
> Never delete entries — append or strike through completed items.

_Last updated: 2026-04-24 — Project initialisation_

---

## Current Phase

**Stage 1 of 5: IR + Pass Manager Core**  
Target duration: 3–4 weeks  
Stage gate: IR can cleanly represent both logical Clifford+T gates AND surface
code patch operations in the same data structure without a lossy conversion.

## Active Story

**None yet — Stage 1 kickoff required**

Next: run `/pm:prd-new` → write Stage 1 PRD → run `/pm:prd-parse` → create
GitHub Issues for Stage 1 epics.

## State of Work

- ✅ Project skeleton created (memory-bank/, docs/, .claude/, scripts/)
- ✅ CLAUDE.md, CHANGELOG.md, all memory-bank files initialised
- ✅ ADR template and index created
- ✅ ccpm skills placed in .claude/skills/ccpm
- 🚧 Stage 1 PRD not yet written
- ⬜ GitHub repo not yet created / Issues not yet set up
- ⬜ CMakeLists.txt not yet written
- ⬜ `QFaultIR` data structures not yet designed
- ⬜ `PassManager` not yet implemented

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
2. Run: ./scripts/quick-test.sh (once CMake is configured)
3. First concrete task: Design QFaultIR data structures
   - Key question: Can one module hold both logical AND physical-level ops?
   - Read: memory-bank/systemPatterns.md section "Two-Level IR"
   - Read: docs/adr/0001-ir-representation.md (once written)
   - Deliverable: src/qfault/ir/QFaultIR.hpp skeleton with Doxygen comments
4. After IR sketch: /pm:prd-new to write Stage 1 PRD formally
5. After PRD: /pm:prd-parse → /pm:epic-decompose → GitHub Issues
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
