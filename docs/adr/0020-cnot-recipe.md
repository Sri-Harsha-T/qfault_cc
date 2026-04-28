# ADR-0020: Logical CNOT recipe — MZZ + SPLIT + MXX + X-meas + Pauli-frame correction

**Status:** Accepted
**Date:** 2026-04-26
**Supersedes:** —

## Context

Logical CNOT in lattice surgery has multiple equivalent constructions
(Horsman 2012 §4.1, Litinski 2019 Pauli-product form, Fowler-Gidney 2018 §XII).
Picking one canonical form and encoding it in `LatticeSurgeryPass` is necessary
so that downstream passes (scheduling, output emission, Stim oracle) see a
uniform pattern.

## Decision

The local CNOT (3-patch L-shape: control C, ancilla A, target T) decomposes to
**4 PatchOps over 2 logical time-steps (= 2d code cycles)**, plus a classical
Pauli-frame update:

```
Step 1 (1τ):   MERGE(C, A, basis=Z)  → outcome m₁ ; SPLIT(C, A)
Step 2 (1τ):   MERGE(A, T, basis=X)  → outcome m₂
End of step 2: MEASURE(A, basis=X)   → outcome m_A

Pauli-frame correction (CLASSICAL, never executed):
    (X_T)^a · (Z_C)^b
where a = m₂ ⊕ m_A, b = m₁ (per Horsman §4.1 derivation).
```

**Total cost: 2d cycles for one logical CNOT.** This is *not* d cycles —
common misreading; the two MERGEs are serialized through the ancilla.

**The IR must produce 4 `PatchOp` instructions (2 MERGE, 1 SPLIT, 1 MEASURE)
plus a classical `PauliFrameUpdate` annotation**, not 4 separate logical CNOT
operations. The `timeStep` field on the second MERGE must be `prev_t + 1`.

The Pauli-frame correction is **tracked classically and absorbed into upcoming
π/8 rotations via GoSC Fig. 4 commutation rules** — never emitted as physical
X/Z gates. This is what makes the cost amortized over the full circuit.

## Alternatives Considered

| Alternative | Why Rejected |
|---|---|
| Litinski Pauli-product form natively | Equivalent but more abstract; harder for first-time IR readers |
| Emit 4 separate logical-level CNOT instructions | Wrong abstraction layer; LOGICAL→PHYSICAL transition is the LatticeSurgeryPass's job |
| Execute Pauli-frame correction as physical gates | Defeats lattice surgery's amortization; doubles physical cost |
| 1-step CNOT via direct three-patch merge | Possible in principle; non-standard; reviewer-confusing |

## Consequences

**Positive:** Single canonical recipe. Stim emission is mechanical
(MERGE→`MZZ`/`MXX`, SPLIT→reset+round, MEASURE→`MX`). Pauli-frame tracker is a
small classical bookkeeping module, isolated from physical IR.

**Negative / Trade-offs:** 2d cost (vs the textbook "1 CNOT in d cycles") must
be documented in marketing; benchmarkers comparing depth must use 2d for
local CNOT, d for long-range (which is also documented).

**Risks:** Forgetting to absorb the Pauli frame into the next π/8 rotation
silently doubles physical T-count. Mitigation: integration test
`tests/integration/test_pauli_frame_absorption` asserts the absorption happens.

## Implementation Notes for AI Sessions

When loading the memory bank:

- Local CNOT cost is **2d cycles**, not d. Long-range CNOT is **1τ regardless
  of length L** — only the spatial footprint scales. Do not multiply time by L.
- The IR emission for one logical CNOT is exactly 4 PatchOps + 1
  PauliFrameUpdate annotation.
- Routing patches in the L-shape need ≥ 1 empty tile gap (boundary-redefinition
  rule from ADR-0019).
- Pauli frame is absorbed into upcoming π/8 rotations via GoSC Fig. 4
  commutation rules. Never emit as physical X/Z.
- For the Stim oracle (ADR-0021), the test is that
  `circuit.has_flow(Z_C → Z_C ⊕ Z_T, X_T → X_C ⊕ X_T)` for both compiled and
  reference circuits.

## References

- Horsman et al. 2012, *Surface code quantum computing by lattice surgery*, §4.1
- Litinski 2019, *A Game of Surface Codes*, Fig. 4 (Pauli-frame commutation), Fig. 7 (T-gate consumption)
- Fowler & Gidney 2018, *Low overhead quantum computation using lattice surgery*, arXiv:1808.06709, §XII