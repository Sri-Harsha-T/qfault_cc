# Stage 7 Spec: Formal-Methods or MLIR Stretch

**Stage:** 7 of 7 (open-ended, optional)
**Target duration:** open-ended (≥ 3 months collaborator commitment required)
**GitHub Milestone:** "Stage 7: Stretch"
**Depends on:** Stage 6 complete OR Stage 5 complete (Stage 6 is itself
optional, so Stage 7 can follow Stage 5 directly)

---

## Goal

Stage 7 is the **OOPSLA / PLDI gating stretch** — without a Stage 7
contribution, ADR-0016 prohibits OOPSLA / PLDI submission. Stage 7 picks
**one** of three mutually-exclusive options based on collaborator interest
and time budget.

The three options were drafted in ADR-0018 (Option A) plus two sibling
drafts referenced from ADR-0009 (Options B and C). Selection happens at
Stage 7 kickoff and is recorded as a follow-up ADR transitioning the
selected option from Draft to Accepted.

**Stage Gate (per option — see "Stage Gate by Option" below):**

> A Stage 7 deliverable that meets the per-option gate criteria and
> licenses an OOPSLA / PLDI submission per ADR-0016.

---

## The three options

### Option A — `qfault.fto` MLIR dialect

Per ADR-0018 (Draft).

**Goal:** Small custom MLIR dialect ("FTO" = Fault-Tolerant Operations)
exposing QFault's IR to the LLVM/MLIR ecosystem. Enables Catalyst /
Quake interop.

**Stage Gate:**
> The `qfault.fto` dialect round-trips a Stage 1+2 LOGICAL-level circuit
> (parsed via Catalyst) through the QFault PassManager and back to MLIR
> with bit-exact equivalence on a 50-circuit regression corpus.

**Deliverables:**
- `mlir/dialects/qfault_fto/` directory tree with TableGen ops, types,
  attributes.
- `qfault-opt` tool wrapping `mlir-opt` with the QFault dialect loaded.
- Round-trip integration tests under `tests/integration/test_mlir_roundtrip.cpp`.
- Pinned LLVM/MLIR release branch (e.g., LLVM 19.x).

**Time budget:** 4–6 months solo; 3 months with an MLIR-experienced
collaborator.

**Risk:** LLVM/MLIR API churn between this stage and the OOPSLA submission.
Mitigated by pinning LLVM and treating the dialect as a thin export
layer, not a core IR.

---

### Option B — Coq-extracted `gridsynth`

**Goal:** A Coq-mechanized Ross-Selinger proof, extracted to OCaml or
directly to compiled code, replacing `GridSynthProvider`'s subprocess
wrapper with a verified-extraction pipeline.

**Stage Gate:**
> The Coq-extracted `gridsynth` produces T-counts within 1% of the
> Haskell reference on a 1000-rotation corpus, AND the Coq proof script
> compiles under Coq 8.18+ without admits, AND the extraction commutes
> through QFault's `SynthesisProvider` Concept boundary with no
> additional adapter code beyond `cmake/coq_extract.cmake`.

**Deliverables:**
- `formal/coq/gridsynth/` directory with `.v` files.
- Extracted artifact under `src/qfault/passes/synthesis/CoqExtractedRS.cpp`.
- New ADR transitioning Option B to Accepted.
- Coq → C++ extraction toolchain documented.

**Time budget:** 6–9 months; requires a Coq-experienced collaborator.

**Risk:** Coq is a steep tax for a research compiler audience; the
mechanized-proof story is OOPSLA-friendly but the maintenance burden is
ongoing.

---

### Option C — Translation-validation harness (most likely to ship)

**Goal:** A per-pass QCEC-obligation harness that validates each pass's
input/output equivalence on every test circuit. Less ambitious than
mechanized verification but **per-pass** rather than end-to-end —
narrows the trust boundary to ~10 passes instead of the entire pipeline.

**Stage Gate:**
> The per-pass QCEC-obligation harness achieves verified equivalence on
> 100% of Stage 1+2 integration-test circuits, with average verification
> time under 10 seconds per pass on a 16-core workstation.

**Deliverables:**
- `verify/translation_validation/` directory with the harness driver.
- A new pass-base member function `[[nodiscard]] virtual
  std::optional<TranslationObligation> obligation() const` (default
  returns `std::nullopt` for passes that don't emit obligations).
- QCEC bridge integration emitting JSON obligations under
  `bench/golden/obligations/<pass>/<circuit>.json`.
- New ADR transitioning Option C to Accepted.

**Time budget:** 3–4 months solo (the lightest of the three options).

**Risk:** Lowest. The harness leans on the existing QCEC bridge from
Stage 2.5. Per-pass obligations are an incremental addition, not a
restructure.

---

## Why this stage exists (rationale)

ADR-0016 (conference target ladder) positions OOPSLA / PLDI as
contingent on a Stage 7 contribution. v0.1's substrate (IR + PassManager
+ reproducibility apparatus) is QCE26 / CGO27 material; it is borderline
for OOPSLA without a "deeper" research contribution. Stage 7 provides
that contribution.

Per ADR-0018 (Draft), MLIR exposure (Option A) preserves the IR design
property that LogicalGate and PatchOp are flat structs, so v0.1 and v0.2
do not foreclose Option A. Per ADR-0009 (verification strategy),
translation validation (Option C) is the closest fit to QFault's
existing testing discipline.

---

## Selection process at Stage 7 kickoff

1. **Read the three Draft ADRs** (ADR-0018 for Option A; sibling drafts
   to be authored at kickoff for B and C).
2. **Confirm collaborator availability** — Option A and Option B are
   substantially harder solo; Option C is feasible solo.
3. **Match to time budget:**
   - 3–4 months → Option C
   - 6–9 months solo → Option C (probably)
   - 4–6 months with MLIR-experienced collaborator → Option A
   - 6–9 months with Coq-experienced collaborator → Option B
4. **Author the selection ADR**: transition the chosen option from Draft
   to Accepted; archive the other two as "Considered, not selected".
5. **Write `docs/phases/stage-7-formal-stretch/exit-criteria-<option>.md`**
   with the option-specific stage gate.

---

## Source Layout (per option)

```
# Option A
mlir/dialects/qfault_fto/
  CMakeLists.txt
  QFaultFTODialect.h
  QFaultFTOOps.td
  QFaultFTOOps.cpp
tools/qfault-opt/
  CMakeLists.txt
  qfault-opt.cpp
tests/integration/
  test_mlir_roundtrip.cpp

# Option B
formal/coq/gridsynth/
  GridProblem.v
  Diophantine.v
  Extract.v
src/qfault/passes/synthesis/
  CoqExtractedRS.cpp
cmake/
  coq_extract.cmake

# Option C
verify/translation_validation/
  Harness.hpp
  Harness.cpp
  ObligationEmitter.hpp
  ObligationEmitter.cpp
include/qfault/passes/
  PassBase.hpp                          # extended with obligation()
bench/golden/obligations/<pass>/<circuit>.json
```

---

## Stage 7 Definition of Done (per option)

### Option A

- [ ] `qfault.fto` dialect builds against pinned LLVM release branch.
- [ ] `qfault-opt` tool runs `--qfault-fto-roundtrip` on 50-circuit
      corpus with bit-exact equivalence.
- [ ] New ADR records LLVM version pin and dialect scope.
- [ ] OOPSLA submission unblocked.

### Option B

- [ ] Coq-extracted `gridsynth` matches Haskell reference within 1%.
- [ ] Coq proof compiles under Coq 8.18+ without admits.
- [ ] Extraction toolchain committed under `formal/coq/`.
- [ ] New ADR records Coq version pin and proof scope.
- [ ] OOPSLA submission unblocked.

### Option C

- [ ] Per-pass obligation harness covers all current passes.
- [ ] Verified equivalence on 100% of Stage 1+2 integration circuits.
- [ ] Average verification time < 10 seconds per pass.
- [ ] New ADR records harness design.
- [ ] OOPSLA / PLDI submission unblocked.

---

## Risks and Mitigations

| Risk | Mitigation |
|------|-----------|
| Stage 7 selection delayed by collaborator search | Option C is feasible solo; default to Option C if no collaborator emerges within 1 month of Stage 7 kickoff |
| LLVM/MLIR churn (Option A) | Pin to a specific LLVM release branch; document version delta in `mlir/VERSION.md` |
| Coq tax (Option B) | Only attempt if a Coq-experienced collaborator commits ≥ 6 months |
| QCEC scaling on large obligations (Option C) | Document per-pass obligation size limits; emit Warn diagnostics for obligations exceeding 8 qubits |
| Stage 7 may not ship | Acceptable. v0.1 + Stage 6 remains a valid milestone; OOPSLA submission is contingent, not required |
