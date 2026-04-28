# Architecture Decision Records — Index

All significant architectural decisions are recorded here.
Before any session proposes a design change, read relevant ADRs.
Use the `/adr` slash command to consult and create ADRs.

> **Rule:** Propose an ADR before: adding a dependency, changing a numerical
> convention, switching synthesis algorithms, or changing the IR schema.

---

## Index

| # | Title | Status | Date | Supersedes |
|---|-------|--------|------|-----------|
| [ADR-0001](0001-ir-representation.md) | IR representation: two-level vs. two-module | Accepted | 2026-04-24 | — |
| [ADR-0002](0002-synthesis-provider-interface.md) | SynthesisProvider as a C++20 Concept | Accepted | 2026-04-25 | — |
| [ADR-0003](0003-global-code-distance.md) | Global code distance d for v0.1; variable-d deferred | Accepted | 2026-04-25 | — |
| [ADR-0004](0004-gridsynth-as-default.md) | GridSynth as default; BFS-table oracle as benchmark baseline | Accepted (with limitation) | 2026-04-25 | partly by ADR-0013 |
| [ADR-0005](0005-qir-version-pinning.md) | QIR output spec version pinning (v0.1 base profile) | Accepted | 2026-04-26 | — |
| [ADR-0006](0006-lattice-surgery-routing.md) | Lattice surgery routing: A* + Litinski templates + earliest-available scheduling | Accepted | 2026-04-26 | — |
| [ADR-0007](0007-msd-factory-selection.md) | MSD factory selection: catalog enumeration with Beverland 2022 cost formulas | Accepted | 2026-04-26 | — |
| [ADR-0008](0008-synthesis-algorithm-portfolio.md) | Synthesis algorithm portfolio: Ross-Selinger now, Kliuchnikov-2023 in Stage 6 | Accepted | 2026-04-26 | — |
| [ADR-0009](0009-verification-strategy.md) | Verification strategy: Stim + MQT QCEC, framed as validation not verification | Accepted | 2026-04-26 | — |
| [ADR-0010](0010-output-backend-portfolio.md) | Output backend portfolio: QASM 3.0 + QIR + Stim native | Accepted | 2026-04-26 | — |
| [ADR-0011](0011-phase-polynomial-zx-pass.md) | Phase-polynomial / ZX optimisation pass: native AMM + optional PyZX bridge | Draft | 2026-04-26 | — |
| [ADR-0012](0012-pybind11-vs-nanobind.md) | Python bindings: pybind11 for v0.1, revisit nanobind for v0.2 | Accepted | 2026-04-26 | — |
| [ADR-0013](0013-skprovider-as-fallback-oracle.md) | Reframing SKProvider as a fallback / sanity oracle (rename to BFSTableProvider) | Accepted | 2026-04-26 | ADR-0004 (in part) |
| [ADR-0014](0014-failed-approach-tracking.md) | Failed-approach tracking as project policy | Accepted | 2026-04-26 | — |
| [ADR-0015](0015-apache2-license.md) | License: Apache 2.0 with explicit patent grant | Accepted | 2026-04-26 | — |
| [ADR-0016](0016-conference-target-ladder.md) | Conference target ladder: QCE26 → CGO27 → optional OOPSLA/PLDI | Accepted | 2026-04-26 | — |
| [ADR-0017](0017-reproducibility-infrastructure.md) | Reproducibility infrastructure: Dockerfile + flake.nix + Zenodo DOI + papers/ | Accepted | 2026-04-26 | — |
| [ADR-0018](0018-mlir-dialect.md) | MLIR exposure via qfault.fto custom dialect (Stage 7 Option A) | Draft | 2026-04-26 | — |

---

## ADR Status Definitions

| Status | Meaning |
|--------|---------|
| **Draft** | Under consideration, not yet decided |
| **Accepted** | Decision made and active |
| **Accepted (with limitation)** | Active but with a known caveat documented in the ADR's Consequences |
| **Deprecated** | Was accepted, now superseded |
| **Superseded** | Replaced by a later ADR (link to replacement) |

---

## Reading order for new contributors

1. **ADR-0001** — the two-level IR design that everything else builds on.
2. **ADR-0002** — the Concept-based SynthesisProvider boundary.
3. **ADR-0014** — the failed-approach tracking policy you'll be touching.
4. **ADR-0015** — the licensing model.
5. **ADR-0009** — the validation-not-verification framing.

Then the stage-specific ADRs as their stages start: **ADR-0006** (Stage 3),
**ADR-0007** (Stage 4), **ADR-0005 + ADR-0010** (Stage 5a), **ADR-0012**
(Stage 5b), **ADR-0008** (Stage 6), **ADR-0018** (Stage 7).

---

## Cross-references

- **CHANGELOG.md "Failed Approaches"** is the runtime companion to ADR-0014.
- **`docs/phases/stage-N/spec.md`** files reference relevant ADRs in their
  "Stage Gate" and "Definition of Done" sections.
- **`memory-bank/activeContext.md`** lists the currently-active ADRs at
  the top of every session via the SessionStart hook.
