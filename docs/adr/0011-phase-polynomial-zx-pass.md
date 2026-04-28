# ADR-0011: Phase-polynomial / ZX optimization pass — Stage 3 native + optional PyZX bridge

**Status:** Draft (target Stage 3 or Stage 6)
**Date:** 2026-04-26
**Supersedes:** —

---

## Context

T-count reduction is the canonical lever for FT-compiler differentiation
against Qiskit's stock pipeline. The two production-grade approaches are:

(a) **Phase-polynomial synthesis** (Amy-Maslov-Mosca 2014; Heyfron-Campbell
    2018) — exact T-count reduction on Clifford+T circuits via polynomial
    parity factorization.

(b) **ZX-calculus rewriting** (PyZX, quizx) — graphical-rewrite-based
    reduction; particularly strong on circuits with phase teleportation
    and CCZ-rich structure.

QFault currently has neither. The "≥20–30% T-count reduction on Feynman
corpus vs Qiskit's stock pipeline" claim depends on at least one of these.

---

## Decision

Stage 3 (or Stage 6, if Stage 3 schedule is tight) ships:

1. **Native C++ Amy-Maslov-Mosca phase-polynomial pass**, ~1000 LOC
   estimate, behind the `PassBase` interface. Always-on. Lives in
   `src/qfault/passes/optimisation/PhasePolyPass.cpp`.

2. **Optional PyZX / quizx bridge** via the pybind11 boundary (Stage 5b
   dependency). Opt-in via CMake flag `-DQFAULT_ENABLE_PYZX_BRIDGE=ON`.
   The bridge calls into the user's installed PyZX/quizx via the Python
   bindings and roundtrips the simplified circuit back through the QASM
   3.0 frontend.

The native pass runs always; the bridge is opt-in and primarily useful
for circuits where ZX rewrites help more than AMM (CCZ-dense workloads,
phase-teleportation patterns).

---

## Alternatives Considered

| Alternative | Why Rejected |
|---|---|
| PyZX only (no native pass) | Forces Python dependency for a core optimization; cuts against C++-native pitch |
| Native ZX in C++ | quizx exists in Rust; reimplementing in C++ is large-scope; bridge is more pragmatic |
| Skip phase-polynomial entirely | Cedes the "≥20–30% T-count reduction vs Qiskit" claim |
| AMM via subprocess wrapper | Same problem as `gridsynth` — process startup dominates per-circuit |

---

## Consequences

**Positive:**
- Native AMM pass licenses the T-count-reduction claim for the arXiv
  preprint and QCE26 submission.
- PyZX bridge adds a second lever for circuits where ZX rewrites help
  more than AMM.
- Both can be benchmarked independently against Qiskit's stock pipeline
  on the Feynman corpus.

**Negative / Trade-offs:**
- ~1000 LOC of new code with a non-trivial correctness story; mitigated
  by golden-circuit tests against PyZX outputs.
- The bridge introduces a Python dependency at runtime when enabled;
  documented as opt-in and CMake-gated.

**Risks:**
- AMM scaling on circuits with thousands of T-gates; mitigated by
  block-decomposition (split into max-1000-T-gate windows, optimize per
  window, recompose).
- Correctness validation: AMM is well-known but the C++ implementation
  must match published reference output. Use Heyfron-Campbell 2018
  reference circuits as ground truth.

---

## Implementation Notes for AI Sessions

When loading the memory bank:
- AMM is the **native pass**; PyZX is **optional**.
- Do NOT depend on PyZX for the headline T-count claim.
- The pass must satisfy `PassConcept` and `requiredLevel() == LOGICAL`.
- For block decomposition, use `PassContext::addDiagnostic(DiagLevel::Info,
  ...)` to log block boundaries — useful for debugging T-count regressions.
- This ADR is **Draft** until Stage 3 kickoff confirms scheduling. If
  Stage 3 schedule is tight, defer to Stage 6 alongside native synthesis.

---

## References

- Amy, Maslov, Mosca 2014, *A meet-in-the-middle algorithm for fast
  synthesis of depth-optimal quantum circuits*
- Heyfron & Campbell 2018, *An efficient quantum compiler that reduces
  T-count*
- PyZX (Kissinger & van de Wetering)
- quizx (Rust port of PyZX)
- ADR-0008 (synthesis algorithm portfolio — adjacent algorithm-portfolio
  lever)
- ADR-0009 (verification strategy — golden-circuit regression)
