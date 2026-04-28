# Stage 7 TODO — Formal-Methods or MLIR Stretch

> Stage 7 is opt-in. The decision to start, defer, or skip is recorded
> in `memory-bank/progress.md`. Once started, exactly one of the three
> options below is pursued; the others are struck through.

## Pre-flight (do before any option starts)

- [ ] Stage 5 closed (v0.1 release tagged, arXiv preprint posted)
- [ ] CGO27 Tools track submission outcome known
- [ ] Collaborator commitment confirmed (≥ 3-month time budget)
- [ ] Option chosen: A / B / C
- [ ] Option-selection ADR written (next sequential number; transitions
      ADR-0018 / Option B draft / Option C draft from Draft to Accepted
      for the chosen option only)

---

## Option A — `qfault.fto` MLIR dialect

(Per ADR-0018.)

### A-1: Toolchain + dialect skeleton

- [ ] Pin LLVM/MLIR release branch in `cmake/dependency_versions.cmake`
      (e.g., `LLVM 19.x`)
- [ ] Add `mlir/dialects/qfault_fto/` directory tree
- [ ] TableGen file `QFaultFTOOps.td` with placeholder ops
- [ ] CMake target `qfault-opt` that links `mlir-opt` + the dialect
- [ ] `qfault-opt --version` works in CI

### A-2: Op definitions

- [ ] `fto.logical_gate` op (mirrors `LogicalGate` IR struct)
- [ ] `fto.patch_op` op (mirrors `PatchOp`)
- [ ] `fto.module` op with `IRLevel` attribute
- [ ] Type definitions: `!fto.logical_qubit`, `!fto.patch_coord`
- [ ] Verifier hooks (assert level invariants on op creation)

### A-3: Round-trip importer/exporter

- [ ] `mlirImporter`: `QFaultIRModule` → `mlir::ModuleOp`
- [ ] `mlirExporter`: `mlir::ModuleOp` → `QFaultIRModule`
- [ ] Round-trip integration test
      `tests/integration/test_mlir_roundtrip.cpp`

### A-4: 50-circuit regression corpus

- [ ] Curate 50 representative circuits (BV, QFT, small Shor instance,
      adder, modular exponentiation pieces)
- [ ] Each circuit: parse → import to MLIR → export → diff against
      original — bit-exact equivalence required
- [ ] CI job `mlir-roundtrip` runs the full corpus on every push

### A-5: Stage gate (Option A)

- [ ] All 50 circuits round-trip with bit-exact equivalence
- [ ] `qfault-opt` documented in `docs/architecture.md`
- [ ] OOPSLA / PLDI artefact eligibility confirmed (per ADR-0016)
- [ ] Exit report at `docs/phases/stage-7-formal-stretch/exit-report.md`

---

## Option B — Coq / Rocq formalisation of one pass

(Per Stage 7 spec § "Option B".)

### B-1: Toolchain + scope decision

- [ ] Pin Coq / Rocq version (e.g., Coq 8.18)
- [ ] Pin Iris version if used
- [ ] Decide: which pass to formalise?
      Recommended: lattice surgery merge/split primitives — the
      smallest pass with non-trivial correctness content.
- [ ] Write the pass contract in Coq: input invariant, output invariant,
      preserved invariants

### B-2: Mechanised proof

- [ ] Stabiliser-formalism preliminaries (Pauli group, code distance,
      surface-code stabilisers)
- [ ] Statement of merge/split correctness
- [ ] Proof skeleton; admitted lemmas first
- [ ] Discharge admitted lemmas one by one
- [ ] All `Admitted.` statements eliminated

### B-3: Connection to QFault C++ implementation

- [ ] Extracted Coq function (or human-translated equivalent) matches
      the C++ implementation behaviour on a regression corpus
- [ ] Document the gap honestly: "the proof is for the abstract algorithm,
      the C++ implementation is checked against the proof on the corpus
      but is not itself extracted"

### B-4: Stage gate (Option B)

- [ ] Coq proof closes (no `Admitted.`, no `Axiom.`)
- [ ] Regression corpus passes
- [ ] Coq sources in `proofs/` directory under Apache 2.0 (per ADR-0015)
- [ ] OOPSLA / PLDI artefact eligibility confirmed
- [ ] Exit report

---

## Option C — Translation validation via SMT

(Per Stage 7 spec § "Option C".)

### C-1: Toolchain

- [ ] Pin SMT solver: Z3 ≥ 4.12 (default) or cvc5 ≥ 1.0 (alternative)
- [ ] Pick the validation scope: which pass(es) get a per-compilation
      equivalence check? Recommended: `LatticeSurgeryPass` (the
      lowering pass — highest semantic gap)

### C-2: Encoder

- [ ] Encode logical-level circuit as SMT (stabiliser semantics)
- [ ] Encode physical-level circuit as SMT (patch-op semantics)
- [ ] Equivalence query: are the two circuits equivalent as logical
      operations (mod stabiliser group action)?

### C-3: Per-compilation validator

- [ ] CLI tool `qfault-validate` that takes a logical circuit + the
      compiler's physical output and emits PASS / FAIL / UNKNOWN
      (Z3 timeout) per compilation
- [ ] Performance budget: ≤ 60 s per circuit on a 100-gate Clifford+T
      benchmark; document the cliff for larger circuits

### C-4: Failure-mode evidence

- [ ] Inject a deliberate compiler bug; confirm `qfault-validate`
      catches it
- [ ] Run on the 50-circuit regression corpus; document UNKNOWN rate

### C-5: Stage gate (Option C)

- [ ] PASS rate ≥ 80 % on the regression corpus
- [ ] At least one deliberate-bug catch demonstrated
- [ ] OOPSLA / PLDI artefact eligibility confirmed
- [ ] Exit report
