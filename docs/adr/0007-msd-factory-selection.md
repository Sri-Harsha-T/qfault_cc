# ADR-0007: MSD factory selection — catalog enumeration with Beverland 2022 cost formulas

**Status:** Accepted
**Date:** 2026-04-26
**Supersedes:** —

---

## Context

Stage 4 must choose magic-state-distillation factories from a catalog and
replicate them to meet T-state demand. Inventing cost formulas would be an
OOPSLA-reviewer red flag; the field has standard catalog entries with
published cost models (Beverland 2022 Appendix C, Bravyi-Haah 2012,
Litinski 2019, Gidney-Fowler 2018).

A note on prior assumptions: the **"116-to-12"** and **"225-to-1"**
factories sometimes attributed to Beverland 2022 are NOT in that paper.
116-to-12 originates in **Bravyi-Haah 2012 (PRA 86, 052329)**, and 225-to-1
appears in derivative Litinski/Microsoft Resource Estimator schemes. This
ADR tracks them as separate sources.

---

## Decision

v0.1 enumerates the catalog
`{Bravyi-Kitaev 15-to-1, Litinski multi-level (15-to-1)², Gidney-Fowler CCZ→2T,
116-to-12, 225-to-1}` with **Beverland 2022 Appendix C cost formulas
verbatim** for the entries it covers (cited inline), and the original
papers for the others. Each catalog entry is a `struct` with a cost
lambda; new entries are additive.

Factory selection is greedy: pick the catalog entry minimizing **spacetime
volume per delivered T-state at the target output error**, then replicate
to meet demand.

The 15-to-1 cost formulas adopted from Beverland Table VI:
- Output T-error rate: `35·p_T³ + 7.1·P_Cliff(d)`.
- Single logical 15-to-1 unit:
  - `N_qubits = 20·n(d) = 40d²` (space-efficient) or `31·n(d)` (RM-prep).
  - Cycle count `C = 13·τ(d)`.
- Logical cycle time: `τ(d) = (4·t_gate + 2·t_meas)·d`.

Two-level cost recurrence:
- `Q_r = 35·Q_{r-1}³ + 7.1·P_r` with `Q_0 = p_T`.
- Total time `τ(D) = Σ τ(M_r)`; total qubits `n(D) = max_r(c_r·n(M_r))`.

Logical error model (per Beverland):
- `P(d) = a·(p/p*)^((d+1)/2)` with `(a, p*) = (0.03, 0.01)` for surface-code
  gate-based, `(0.08, 0.0015)` for surface-code Majorana
  measurement-based, `(0.07, 0.01)` for Hastings-Haah.
- Code-distance selection: `d = ⌈2·log(aε/3QC) / log(p*/p) − 1⌉`
  rounded to next odd.

Practical threshold: single-level suffices for `P_T ≳ 10⁻¹⁰`; below `10⁻¹²`
concatenation is required.

---

## Alternatives Considered

| Alternative | Why Rejected |
|---|---|
| Inventing cost formulas | Unverifiable; reviewer red flag |
| ILP factory placement | Optimization belongs to Stage 4.5 or later; v0.1 needs reproducible reference numbers |
| Single-factory always (BK 15-to-1 only) | Cannot reproduce Beverland 2022 estimates that use multi-level factories |
| Beverland-baseline only (no CCZ→2T) | Misses a 25%-smaller / 2× rate / "halved T-count on Toffoli-dominated workloads" improvement; Gidney-Fowler must be in the catalog |

---

## Consequences

**Positive:**
- Numbers are checkable against Beverland 2022 Tables 1–3 directly;
  reviewer can reproduce by hand.
- Catalog extension is additive (new entry → new struct + cost lambda).
- Three Beverland Table VII assembled factories covered:
  - 15-to-1 SE (×1, d=9) → P_T ≈ 5.6×10⁻¹¹, 3,240 qubits, 46.8 µs.
  - 15-to-1 SE (×16, d=5) → 15-to-1 RM-prep (d=13) → 2.1×10⁻¹⁵,
    16,000 qubits, 83.2 µs.
  - 15-to-1 SE (×16, d=3) → 15-to-1 SE (d=11) → 5.51×10⁻¹³,
    5,760 qubits, 72.8 µs.

**Negative / Trade-offs:**
- Greedy selection is suboptimal on circuits with mixed-precision T-state
  demand; documented as a known limitation.
- Beverland's `+7.1` and `+356` leakage coefficients are explicitly hedged
  in the source as numerical estimates; QFault must propagate the same
  hedging in `ResourceReport` output.

**Risks:**
- Beverland 2022's formulas assume specific physical-error rates and
  cycle times; QFault must propagate these as `PassContext` fields and
  reject configurations outside the validated regime
  (`p ∈ {10⁻³, 10⁻⁴}`, cycle ≈ 1 µs) with a runtime warning.

---

## Implementation Notes for AI Sessions

When loading the memory bank:
- Factory cost formulas come from Beverland 2022 Appendix C (15-to-1
  variants) and the original papers for Bravyi-Haah 116-to-12,
  Gidney-Fowler CCZ→2T, and Litinski/Microsoft 225-to-1. **Never invented.**
- Each factory is a struct with a cost lambda. Catalog file:
  `include/qfault/passes/msd/FactoryCatalog.hpp`.
- Do NOT implement ILP placement in v0.1.
- When CCZ→2T catalysis is selected for Toffoli-heavy workloads, the
  effective T-count halves vs Beverland baseline; document this
  prominently in `ResourceReport` output.

---

## References

- Beverland et al. 2022, *Assessing requirements to scale to practical
  quantum advantage*, Appendix C, Tables VI–VII
- Bravyi & Kitaev 2005, *Universal quantum computation with ideal Clifford
  gates and noisy ancillas*
- Bravyi & Haah 2012, PRA 86, 052329 (116-to-12 origin)
- Gidney & Fowler 2019, *Efficient magic state factories with a catalyzed
  |CCZ⟩ to 2|T⟩ transformation* (arXiv:1812.01238)
- Litinski 2019, *Magic State Distillation: Not as Costly as You Think*
- ADR-0003 (global-d limitation; Stage 4 may need variable-d follow-up
  ADR)
- ADR-0006 (lattice surgery routing — feeds factory placement)
