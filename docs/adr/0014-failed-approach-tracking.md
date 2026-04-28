# ADR-0014: Failed-approach tracking as project policy

**Status:** Accepted
**Date:** 2026-04-26
**Supersedes:** —

---

## Context

The `CHANGELOG.md` "Failed Approaches" table currently documents five
concrete dead ends from Stage 1 and Stage 2:

1. gcc-13 `-Wmissing-field-initializers` for designated-init aggregates
   omitting `std::optional` fields.
2. UBSAN signed-integer-overflow at `i=65536` in a `volatile int sum`
   spin loop.
3. System-gcc-9.4 trap that broke a generic `debug` preset because gcc-9.4
   cannot compile C++20 default `operator==`.
4. The corresponding `quick-test.sh` hardcoded-path bug.
5. The "no T/Tdg remain after `TGateSynthesisPass<SKProvider>`" assertion
   that misread `SKProvider` semantics (see ADR-0013).

This is unusual project hygiene; formalizing it as policy strengthens the
reproducibility story and prevents AI sessions from retrying known dead
ends.

---

## Decision

Failed approaches are tracked in `CHANGELOG.md` under a stable
**"Failed Approaches"** heading. Each entry has:

(a) the approach attempted (brief description)
(b) the failure mode (compile error, sanitizer report, semantic mismatch,
    test assertion)
(c) the root cause (be specific — error code, line, version)
(d) the surrounding context (compiler version, stage, pass, commit hash
    if applicable)
(e) the alternative that replaced it

Entries are **append-only and never deleted**. A new failed approach is
added by the `/log` slash command (`.claude/commands/log.md`). The arXiv
preprint and conference paper cite this table as evidence of
reproducibility discipline.

The table format is fixed:

```
| Date | Approach | Why It Failed | Alternative Used |
|------|----------|---------------|------------------|
```

---

## Alternatives Considered

| Alternative | Why Rejected |
|---|---|
| Track failed approaches in commit messages only | Diffuses the signal; reviewers cannot find them |
| Track in an issue tracker | Loses the source-tree-local reproducibility property |
| Don't track | Forfeits a real reproducibility-evaluation differentiator |
| Track in per-stage exit reports only | Per-stage scope is too narrow; cross-stage learnings are lost |

---

## Consequences

**Positive:**
- AI sessions loading the memory bank cannot accidentally retry known
  dead ends — the table is read at every `/resume`.
- Reviewers see a project that knows what doesn't work, not just what
  does.
- Aligns with ACM artifact evaluation **Reusable** criteria: "the
  artifacts associated with the paper are well-documented enough to be
  reused by other researchers."

**Negative / Trade-offs:**
- Modest documentation overhead per failed approach; mitigated by the
  `/log` slash command which does the formatting.

**Risks:**
- None substantive. The table grows monotonically; if it grows beyond
  ~30 entries, split into per-stage archives (deferred until that
  threshold is reached).

---

## Implementation Notes for AI Sessions

When loading the memory bank, read `CHANGELOG.md` "Failed Approaches"
**before proposing any approach** in `cpp.md` or `qec.md` scope. If an
approach matches a logged failure, use the documented alternative instead
and add a back-reference to the existing entry rather than a duplicate.

When adding a new entry via `/log`:
- Today's date in `YYYY-MM-DD`.
- The approach in 1–2 plain sentences.
- The failure mode with a specific error/diagnostic where possible.
- The replacement, including a commit hash if the fix is committed.

The `/log` slash command also updates `memory-bank/activeContext.md` with
the latest failed approach so it appears at session start.

---

## References

- `CHANGELOG.md` "Failed Approaches" section (live table)
- `.claude/commands/log.md` (slash command implementation)
- ACM artifact evaluation guidelines
- ADR-0013 (where the SKProvider misread was promoted to a design note)
- ADR-0017 (reproducibility infrastructure — failed-approach log is part
  of the artifact bundle)
