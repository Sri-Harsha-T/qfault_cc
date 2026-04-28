# ADR-0016: Conference target ladder — QCE26 → CGO27 → optional OOPSLA/PLDI

**Status:** Accepted
**Date:** 2026-04-26
**Supersedes:** —

---

## Context

A research compiler benefits from venue-aligned framing. QFault's
substrate (compiler infrastructure, IR, passes, reproducibility apparatus)
maps cleanly onto compiler venues; the FT-quantum domain maps onto IEEE
Quantum Week (QCE).

The team must choose a sequenced submission ladder rather than scatter-shot
submissions, both to manage reviewer risk and to align Stage 5c, Stage 6,
and Stage 7 deliverables with venue deadlines.

---

## Decision

The submission ladder is **QCE26 → CGO27 → optional OOPSLA/PLDI**.

**QCE26 first** (~6 months prep):
- Lowest reviewer-risk venue for v0.1.
- Industry track tolerates engineering-heavy submissions.
- Deadline ≈ mid-2026.
- Aligns with Stage 5c (arXiv preprint + artifact).
- Frame: end-to-end FT compiler pipeline + reproducibility apparatus.

**CGO27 Tools track second** (~12 months prep):
- Compiler-infrastructure focus matches QFault's substrate.
- ACM artifact evaluation rewards the MADR-lite + CCPM + memory-bank
  + failed-approach-log discipline directly.
- Aligns with Stage 6 (native synthesis) and target the triple-badge.
- Frame: C++20 Concepts + variant IR + pass-manager design lessons.

**OOPSLA or PLDI optional** (~18+ months prep):
- Gated on Stage 7 formal-methods or MLIR stretch (ADR-0018).
- Higher reviewer-risk without Stage 7 contribution; do NOT submit
  without one of: translation-validation harness (Option C), Coq
  extraction (Option B), or MLIR dialect (Option A).

Each submission requires a frozen `papers/<venue>/` directory with:
- `paper.tex` and `figures/` per venue style.
- `make figures` target reproducing all plots from `bench/`.
- Pinned input data hashes in `papers/<venue>/inputs.lock`.
- Zenodo DOI minted at submission.

---

## Alternatives Considered

| Alternative | Why Rejected |
|---|---|
| OOPSLA first | Higher reviewer-risk without Stage 7 stretch; v0.1 substrate alone is borderline |
| QIP first | Theory-heavy venue; QFault is engineering-heavy |
| Workshops only (e.g. PLDI-affiliated) | Lower visibility; misses artifact-evaluation reward |
| arXiv only, no peer-reviewed venue | Misses the credentialing benefit; community reviewers want a venue stamp |
| Multiple parallel submissions | Risk of conflicting acceptance / reviewer overlap; sequence is safer |

---

## Consequences

**Positive:**
- Sequenced ladder reduces reviewer risk and aligns Stage 5c, Stage 6,
  and Stage 7 deliverables with venue deadlines.
- QCE first means engineering rigor takes precedence over theoretical
  novelty — aligned with QFault's strengths.
- ACM artifact evaluation (CGO/OOPSLA) rewards the reproducibility
  apparatus that already exists; the marginal cost of submitting is low
  once `papers/qce-2026/` is built.

**Negative / Trade-offs:**
- QCE first means the first venue is community-recognized but not
  CS-systems-tier; mitigated by CGO27 follow-up.
- Three submissions means three frozen artifact bundles to maintain;
  mitigated by `papers/<venue>/` isolation.

**Risks:**
- Venue deadlines may slip; mitigated by treating each submission as
  independent and by allowing 2–3 month deadline buffer in Stage 5c
  planning.
- Reviewer overlap between QCE and CGO is low (different communities);
  not a major concern.

---

## Implementation Notes for AI Sessions

When loading the memory bank:
- Target **QCE26 first**. Do NOT submit to OOPSLA/PLDI without Stage 7
  deliverables.
- Each `papers/<venue>/` is frozen at submission — no edits after the
  Zenodo DOI is minted.
- Use `make figures-tier1`, `make figures-tier2`, `make figures-tier3`
  in `bench/Makefile` to reproduce per-tier plots; the QCE26 paper uses
  Tier 1 + Tier 2; CGO27 uses Tier 1 + Tier 2 + Tier 3.

---

## References

- QCE 2026 Call for Papers (Industry Papers track)
- CGO 2027 Tools track CFP
- ACM artifact evaluation guidelines
- ADR-0017 (reproducibility infrastructure — papers/ directory layout)
- ADR-0018 (Stage 7 stretch — gates OOPSLA/PLDI submission)
