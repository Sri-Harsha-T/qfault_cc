# Stage 7 Kickoff — Formal-Methods or MLIR Stretch

**Stage:** 7 of 7 (open-ended, optional)
**Milestone:** "Stage 7: Stretch"
**Prerequisite:** Stage 5 (or Stage 6) complete; OOPSLA / PLDI submission ambition declared.

---

## Why this stage exists

ADR-0016 (Conference Target Ladder) gates OOPSLA / PLDI submission on a
Stage 7 deliverable. Without Stage 7, the project is restricted to QCE26
and CGO27 (Tools track). Stage 7 is the dial-mover that elevates QFault
from "useful tool" (CGO Tools track) to "novel research artefact"
(OOPSLA / PLDI main track).

This is **optional**. Skipping Stage 7 is a valid project completion
decision — the v0.1 → v0.2 trajectory through QCE26 + CGO27 is itself
a respectable academic deliverable. Stage 7 is opt-in based on
collaborator interest and a credible ≥ 3-month time budget.

---

## Decision gate at kickoff

Stage 7 picks **one** of three mutually-exclusive options. The decision
is recorded as a follow-up ADR transitioning the selected option from
Draft to Accepted.

| Option | What it produces | Time budget | Collaborator profile |
|--------|-----------------|-------------|---------------------|
| **A — MLIR dialect** | `qfault.fto` dialect + `qfault-opt` tool + round-trip tests | 3–6 months | C++ / LLVM / MLIR systems engineer |
| **B — Coq formalisation** | Mechanised proof of one pass (lattice surgery primitives) | 4–8 months | Coq / Iris / Rocq researcher |
| **C — Translation validation** | Per-compilation SMT-based equivalence checker | 3–5 months | SMT / verification practitioner |

**Choosing criteria:**
1. Whose CV does this serve? Industry roles favour Option A; research
   PhD / postdoc roles favour Option B; security-adjacent roles favour
   Option C.
2. What collaborator is reachable? Coq formalisation without a
   Coq-fluent collaborator is a 12+ month dead end; do not start it
   without one.
3. What does the v0.1 portfolio actually need? If conference reviewers
   in the CGO submission flagged "no novelty in routing," Option A or
   Option C address that. If they flagged "no correctness guarantee,"
   Option B addresses that.

---

## Pre-flight checklist

Before writing the option-selection ADR:

- [ ] Stage 5 closed (v0.1 release, arXiv preprint posted, ≥ 1 community
      response logged in `memory-bank/progress.md`).
- [ ] Optional Stage 6 outcome decided (closed, deferred, or not started).
- [ ] CGO27 Tools track submission outcome known (accepted, rejected,
      or pending). If pending, defer Stage 7 kickoff until decision
      received — reviewer feedback informs option choice.
- [ ] Collaborator commitment in writing (email, signed MoU, or GitHub
      issue assignee).
- [ ] Funding / time budget realistic (Stage 7 is ≥ 3 months on top of
      everything else; do not start with < 3 months runway).

---

## What this stage is not

- **Not a fallback for Stage 5 not converging.** If v0.1 is shaky,
  fix v0.1, do not pivot to Stage 7.
- **Not a substitute for community engagement.** Even a Stage 7
  contribution does not replace the QCE26 / CGO27 submission ladder
  in ADR-0016.
- **Not a multi-option stage.** Picking two of {A, B, C} is a
  scope-creep failure mode — each option alone is already a 3+ month
  deliverable.

---

## Reading order at kickoff

1. ADR-0016 (Conference Target Ladder) — re-confirms why Stage 7 exists
2. ADR-0018 (MLIR dialect, Draft) — Option A
3. CGO27 reviewer feedback (if available)
4. arXiv preprint reception (community response in
   `memory-bank/progress.md`)
5. The collaborator's CV / interest profile
6. This document

Then write `docs/adr/00NN-stage-7-option-selection.md` recording the
decision before writing any code.
