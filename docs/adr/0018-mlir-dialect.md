# ADR-0018: MLIR exposure via qfault.fto custom dialect

**Status:** Draft (Stage 7 stretch, optional)
**Date:** 2026-04-26
**Supersedes:** —

---

## Context

PennyLane Catalyst, NVIDIA Quake, and increasingly the broader
quantum-compiler ecosystem use MLIR. A small `qfault.fto` custom dialect
("FTO" = Fault-Tolerant Operations) would enable Catalyst/Quake interop
and unlock an MLIR-flavored framing for OOPSLA / PLDI submission.

LLVM/MLIR version churn is a known time-sink for solo maintainers — the
LLVM project releases every 6 months and minor MLIR API breaks are
routine. Committing to MLIR before v0.1 ships would dominate Stage 1+2
schedule with no v0.1 user-visible benefit.

---

## Decision

**Defer to Stage 7** as one of three mutually-exclusive stretch options
(see ADR-0018 sibling drafts for Options B and C):

- **Option A (this ADR):** small `qfault.fto` MLIR dialect for
  Catalyst/Quake interop.
- **Option B:** Coq-extracted gridsynth for a verified-extraction story.
- **Option C:** translation-validation harness emitting QCEC obligations
  per pass (currently the most-likely-to-ship option per ADR-0009
  alignment).

v0.1 and v0.2 keep the upgrade path open by avoiding any IR design that
would conflict with a future MLIR mapping. Specifically, the existing
`std::variant<LogicalGate, PatchOp>` IR maps cleanly onto MLIR-style
operation registries (`Operation*` + reflective casting); flat structs
without class hierarchies are MLIR-compatible.

If Stage 7 selects Option A:
- Pin LLVM/MLIR to a specific release branch (e.g. LLVM 19.x).
- Build the dialect under `mlir/dialects/qfault_fto/`.
- Keep the dialect as a **thin export layer**, not a core IR replacement.
- Provide a `qfault-opt`-style tool that converts QFaultIR ↔ qfault.fto.

---

## Alternatives Considered

| Alternative | Why Rejected |
|---|---|
| Adopt MLIR as the primary IR in v0.1 | LLVM/MLIR version churn would dominate Stage 1–2 schedule |
| Reject MLIR entirely | Forecloses a real interop path with Catalyst/Quake |
| Adopt MLIR only at backend emission | Ambiguous benefit; deferring is cheaper |
| QIR-as-MLIR via QIR Alliance dialect | QIR Alliance MLIR dialect is itself unstable; would inherit two stability problems |

---

## Consequences

**Positive:**
- Upgrade path preserved without v0.1 schedule cost.
- If Stage 7 selects Option A, the existing IR maps with low friction
  because `LogicalGate` and `PatchOp` are flat structs.
- Aligns with the "compiler infrastructure" framing for OOPSLA / PLDI
  submission (per ADR-0016).

**Negative / Trade-offs:**
- No MLIR audience benefit until Stage 7. PennyLane Catalyst /
  NVIDIA Quake users see QFault as a non-MLIR tool until then.

**Risks:**
- MLIR API churn between now (April 2026) and Stage 7 kickoff;
  mitigated by treating the dialect as a thin export layer, not a core
  IR.
- Solo maintainer time-sink is the main reason this is **Draft** and
  optional. Stage 7 selection requires explicit user/collaborator
  commitment of ≥3 months.

---

## Implementation Notes for AI Sessions

When loading the memory bank:
- Do NOT introduce MLIR dependencies before Stage 7 kickoff.
- Preserve the IR design property that `LogicalGate` and `PatchOp` are
  flat structs with no class hierarchy.
- If Stage 7 selects Option A, this ADR moves from Draft to Accepted,
  and a new sibling ADR records the LLVM version pin.
- Options B and C have separate Draft ADRs (deferred until Stage 7
  selection); don't conflate them with this one.

---

## References

- MLIR documentation: https://mlir.llvm.org/
- PennyLane Catalyst MLIR dialect
- NVIDIA Quake dialect
- VOQC (Hietala et al.) — relevant if Option B is selected
- ADR-0009 (verification strategy — Option C aligns)
- ADR-0016 (conference ladder — gates OOPSLA/PLDI on Stage 7 selection)
