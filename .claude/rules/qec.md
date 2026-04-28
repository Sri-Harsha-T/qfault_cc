---
paths:
  - "src/qfault/passes/**"
  - "include/qfault/passes/**"
  - "src/qfault/ir/**"
  - "include/qfault/ir/**"
  - "verify/**"
  - "tests/integration/**"
---

# QEC Domain Rules for QFault — apply when touching pass / IR / verify code

These rules apply to QEC-specific source files. Read `docs/glossary.md` if
any term below is unfamiliar. These rules encode quantum-error-correction invariants that the type system does not enforce. They are *load-bearing*: violating them silently produces compiled code that decodes incorrectly. Read in conjunction with `.claude/rules/cpp.md` and (for Stage 3) `.claude/rules/routing.md`.

## Code distance invariants (ADR-0003)

- **Code distance `d` must be odd and ≥ 3.** Encoded as runtime check in
  `PassContext` constructor — wrong distances cannot exist. If you touch
  `PassContext`, do not relax this assertion.
- **Single global `d` through Stage 3.** Variable-d (region-keyed) is a
  Stage 4 deliverable. Do not retrofit `std::map<RegionId, int>` distance
  in Stage 1–3 code without a new ADR.
- **Physical qubits per patch at distance d**: `2d² − 1` (`d²` data + `d²−1`
  measure ancillas). For d=5: 49 qubits. For d=11: 241. For d=13: 337.

## Stabiliser and Pauli Conventions

- Pauli operators: `I=0, X=1, Y=2, Z=3` — use the `Pauli` enum class.
- **Pauli enum values are `I=0, X=1, Y=2, Z=3`.** This matches Stim's tableau convention and de-risks Stage 2.5+ Stim integration. Do not reorder.
- Measurement basis: `X` or `Z` only in surface code (no Y measurements directly).
- Stabiliser phases: always +1 for surface code stabilisers (no -1 generators).
- Logical operators: X-type is a row of X operators; Z-type is a column — confirm orientation is consistent with `PatchOrientation` in the current patch layout.
- **Surface code is rotated only** (not toric, not colour code). A `--code-family`
  flag for v0.2+ is reserved.
- **MeasBasis enum has only X and Z.** `MeasBasis::Y` is **deliberately
  excluded**: Y measurement is composite in the surface code and would require
  patch-deformation gymnastics. If you find yourself wanting Y, the answer is
  to decompose into Clifford + X/Z measurement at the IR level, not to add
  `MeasBasis::Y`.

## Boundary conventions (ADR-0019)

- **Dashed boundaries are X-rough, solid boundaries are Z-rough** (GoSC convention).
- **Default patch orientation:** Z_L vertical, X_L horizontal.
- **MERGE pairs matching boundaries only:** X-edge ↔ X-edge yields MXX;
  Z-edge ↔ Z-edge yields MZZ. **Mixing X with Z is a logic error** —
  `MergeOp::validate()` asserts at construction. Do not add a `force_mixed_merge`
  bypass; if a real algorithm requires it, it requires a new ADR.
- **MYY without twist defects is forbidden in v0.1.** Reserved for v0.2+ via
  `--use-twists` flag.

## CNOT semantics (ADR-0020)

- **Local CNOT cost is 2d cycles, not d.** The L-shape requires two MERGEs
  serialized through an ancilla.
- **Long-range CNOT is 1τ regardless of length L.** Only the spatial footprint
  scales linearly. Do not multiply time cost by L.
- **The IR emits 4 PatchOps + 1 PauliFrameUpdate** for one logical CNOT,
  not 4 separate logical CNOT operations.
- **Pauli-frame correction is classical**, never executed as physical X/Z gates.
  Absorbed into upcoming π/8 rotations via GoSC Fig. 4 commutation rules.
- An integration test
  (`tests/integration/test_pauli_frame_absorption`) asserts the absorption
  happens; do not skip-flag it without a reason and a CHANGELOG entry.

## Patch geometry rules

- **Adjacent data patches require ≥ 1 empty tile gap** (otherwise their
  boundaries cannot be redefined dynamically).
- **One τ ≈ d code cycles** (one syndrome-extraction round).
- **One tile = d² physical data qubits** (≈ 2d² with measure ancillas).
- **Routing patches** are ancilla snakes occupying L tiles for length-L route.

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

## Lattice surgery: factory cost rules (ADR-0007)

- **Beverland 2022 Appendix C contains 15-to-1 protocols only.**
  - "116-to-12" originates in Bravyi-Haah 2012 (PRA 86, 052329)
  - "225-to-1" appears in Litinski 2019 / Microsoft RE derivative schemes
  - Do not cite either as a "Beverland factory" without separate citation
- **Beverland's three actually-used two-level factories** (Table VII):
  P_T = 5.6×10⁻¹¹ (3,240 qubits / 46.8 µs);
  P_T = 2.1×10⁻¹⁵ (16,000 qubits / 83.2 µs);
  P_T = 5.51×10⁻¹³ (5,760 qubits / 72.8 µs).
- **Logical error model:** `P(d) = a·(p/p*)^((d+1)/2)` with `(a, p*) = (0.03, 0.01)`
  for surface-code gate-based, `(0.08, 0.0015)` for surface-code Majorana
  measurement-based, `(0.07, 0.01)` for Hastings-Haah.
- **Anchor sweeps at p = 10⁻³ (conservative) and p = 10⁻⁴ (realistic).**
  Cycle time 1 µs near-term superconducting. These match all downstream
  Gidney-Ekerå/Babbush/Beverland numbers.

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
- **Synthesis ε is read from `PassContext`, never hardcoded.**
  Default: `1e-10` for `GridSynthProvider`. `BFSTableProvider`'s achievable
  accuracy is **≈ 1e-3**, so its tests use `1e-3` tolerance — that is correct, not a bug. Do NOT hardcode it.
- T-count for common gates (validate against GridSynth paper, Table 1):
  - T: trivially 1
  - Rx(π/4): ~3 T-gates at ε=1e-10
  - Generic SU(2): log₂(1/ε) × O(1) T-gates
- If a synthesised gate sequence has T-count > 3× the GridSynth reference,
  log a warning via `PassContext::addDiagnostic(DiagLevel::Warn, ...)`.
- **`SKProvider`'s `eps` argument is effectively ignored** beyond use as a
  tolerance check downstream. Document this when reviewing related code; do
  not "fix" it by deepening the BFS — combinatorial explosion (logged Failed
  Approach context, ADR-0013).

## Validation, not verification (ADR-0009)

- Use the word **"validated"** not "verified" in headers, comments, and
  documentation.
- The CI gate is **Stim equivalence + MQT QCEC equivalence + golden tests**,
  not formal proof.
- "Verification" is reserved for the optional Stage 7 formal-methods stretch.
  Do not use the word elsewhere.

## Stim Oracle Integration Rules (ADR-0021)

- **Stim version pinned to v1.15.0.** Do not bump without a new ADR.
- **Library target is `libstim`** (not `stim` — that's the CLI binary).
- **SIMD width pinned to 64** via `-DSIMD_WIDTH=64` for golden reproducibility.
- **Stim is private to `verify/` and `tests/`.** Never include in public headers.
- **Equivalence primary: `Circuit::has_flow(Flow)`.** Backstop: noiseless
  detector-distribution comparison after `circuit.without_noise()`.
- **Forbidden Stim patterns:** empty `REPEAT(0)`; first-round detectors
  referencing prior rounds; `OBSERVABLE_INCLUDE` referencing detectors;
  `detector_error_model()` on noiseless circuits.
- `scripts/compare-stim.sh` is the golden test. If it fails, the compiler is wrong.
- When writing integration tests, always include a Stim reference output in
  `tests/reference/` committed as a golden file.
- Do NOT modify reference files without explicit review — they are ground truth.
- Stim command to run a stabiliser circuit: `stim sample --shots N < circuit.stim`

## Numerical convention rules

- **Frobenius distance** for synthesis quality, modulo global phase.
- **Wall time** in microseconds (`std::chrono::microseconds`) for timing.
- **Physical qubit counts** as `std::size_t`.
- **Spacetime volume** in units of d³ (tile-cycles), as `std::int64_t` (signed
  for UBSAN overflow detection).

## When to require an ADR

Propose an ADR before:

- Adding a new external dependency (Stim, MQT QCEC, MPFR, etc.)
- Changing a numerical convention (Pauli enum values, angle units, ε defaults)
- Switching synthesis algorithms or adding a third one
- Changing the IR schema (new variant alternative, new `IRLevel`)
- Bumping pinned versions of Stim, QCEC, GridSynth, QIR
- Relaxing a runtime assertion in `PassContext` or `MergeOp::validate()`
- Adding `MeasBasis::Y` (won't be approved for v0.1; document the reason
  in the new ADR if you propose it)
