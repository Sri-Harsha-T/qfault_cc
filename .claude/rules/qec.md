---
paths:
  - "src/qfault/passes/lattice/**"
  - "src/qfault/passes/msd/**"
  - "src/qfault/passes/synthesis/**"
  - "src/qfault/ir/**"
---

# QEC Domain Rules for QFault

These rules apply to QEC-specific source files. Read `docs/glossary.md` if
any term below is unfamiliar.

## Stabiliser and Pauli Conventions

- Pauli operators: `I=0, X=1, Y=2, Z=3` — use the `Pauli` enum class.
- Measurement basis: `X` or `Z` only in surface code (no Y measurements directly).
- Stabiliser phases: always +1 for surface code stabilisers (no -1 generators).
- Logical operators: X-type is a row of X operators; Z-type is a column — confirm
  orientation is consistent with `PatchOrientation` in the current patch layout.

## Surface Code Conventions

- **Rotated surface code** is the target (not toric code, not unrotated).
- Data qubits at integer `PatchCoord`; measure qubits at half-integer NOT used —
  use a separate `MeasQubit` type or index.
- Code distance `d` is always odd and ≥3. Assert this at runtime.
- Physical qubit count per patch: `d² data + (d²-1) measure = 2d²-1` qubits.
- **Assert `d % 2 == 1`** whenever `d` is used.

## Lattice Surgery Rules

- A MERGE always pairs an X-boundary of one patch with an X-boundary of another
  (for logical-X measurement) or Z with Z (for logical-Z measurement).
- Never merge incompatible boundaries (X with Z) — this is a logical error, not
  a code error. Add a runtime assertion.
- A SPLIT always follows a MERGE in the same code cycle or the next — orphaned
  MERGEs are a protocol error. Validate in `LatticeSurgeryPass::verify()`.
- Routing: bus patches are ancilla patches in `IDLE` state between data patches.
  Each hop adds 1 code cycle to the MERGE latency.

## Magic State Distillation Rules

- A 15-to-1 factory footprint is approximately `(4d+1) × (8d+1)` data patches
  at code distance `d`. Use this as the default factory size unless a different
  protocol is specified.
- T-gate latency = factory cycle time + Manhattan routing distance (in code cycles).
- Never schedule two T-gates to the same factory in the same time slot.
- `MSDSchedulerPass` must produce a factory schedule that Stim can simulate
  without modification — validate this in integration tests.

## Synthesis Rules

- GridSynth is the default provider (ADR-0004). Do NOT change this default.
- The synthesis approximation parameter ε = 1e-10 by default (from `PassContext`).
  Do NOT hardcode it.
- T-count for common gates (validate against GridSynth paper, Table 1):
  - T: trivially 1
  - Rx(π/4): ~3 T-gates at ε=1e-10
  - Generic SU(2): log₂(1/ε) × O(1) T-gates
- If a synthesised gate sequence has T-count > 3× the GridSynth reference,
  log a warning via `PassContext::addDiagnostic(DiagLevel::Warn, ...)`.

## Stim Oracle Integration

- `scripts/compare-stim.sh` is the golden test. If it fails, the compiler is wrong.
- When writing integration tests, always include a Stim reference output in
  `tests/reference/` committed as a golden file.
- Do NOT modify reference files without explicit review — they are ground truth.
- Stim command to run a stabiliser circuit: `stim sample --shots N < circuit.stim`
