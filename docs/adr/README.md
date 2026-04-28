# Architecture Decision Records — Index

All significant architectural decisions are recorded here.
Before any session proposes a design change, read relevant ADRs.
Use the `/adr` slash command to consult and create ADRs.

> **Rule:** Propose an ADR before: adding a dependency, changing a numerical
> convention, switching synthesis algorithms, or changing the IR schema.

The MADR-lite house style (see ADR-0001 for the canonical example): header,
three-line front matter (`Status`, `Date`, `Supersedes`), `Context | Decision |
Alternatives Considered (table) | Consequences (Positive / Negative–Trade-offs / Risks) |
Implementation Notes for AI Sessions | References`.
---

## Index

| # | Title | Status | Date | Stage | Supersedes |
|---|-------|--------|------|-------|-----------|
| [ADR-0001](0001-ir-representation.md) | IR representation: two-level via `variant<LogicalGate, PatchOp>` | Accepted | 2026-04-25 | 1 | — |
| [ADR-0002](0002-synthesis-provider-interface.md) | `SynthesisProvider` as a C++20 Concept | Accepted | 2026-04-25 | 2 | — |
| [ADR-0003](0003-global-code-distance.md) | Global code distance `d` for v0.1 | Accepted | 2026-04-25 | 2 | — |
| [ADR-0004](0004-gridsynth-as-default.md) | GridSynth default; BFS-table oracle as benchmark baseline | Accepted (with limitation) | 2026-04-25 | 2 | — (partial supersession by ADR-0013) |
| [ADR-0005](0005-qir-version-pinning.md) | QIR Alliance v0.1 base profile pinned | Accepted | 2026-04-26 | 5a | — |
| [ADR-0006](0006-routing-algorithm.md) | Lattice surgery routing: A* on tile grid + Litinski templates + Silva 2024 EAF | Accepted | 2026-04-26 | 3 | — |
| [ADR-0007](0007-msd-factory-catalog.md) | MSD factory selection: Beverland 2022 catalog + Gidney-Fowler CCZ→2T | Accepted | 2026-04-26 | 4 | — |
| [ADR-0008](0008-synthesis-portfolio.md) | Synthesis algorithm portfolio: Ross-Selinger now, Kliuchnikov-2023 in Stage 6 | Accepted | 2026-04-26 | 6 | — |
| [ADR-0009](0009-validation-not-verification.md) | Verification strategy: validation (Stim + QCEC), not formal verification | Accepted | 2026-04-26 | 2.5 | — |
| [ADR-0010](0010-output-backends.md) | Output backend portfolio: QASM 3.0 + QIR + Stim native | Accepted | 2026-04-26 | 5a | — |
| [ADR-0011](0011-phase-polynomial-pass.md) | Phase-polynomial / ZX optimization pass — native + optional PyZX bridge | Draft | 2026-04-26 | 3 or 6 | — |
| [ADR-0012](0012-pybind11-for-v01.md) | Python bindings: pybind11 v0.1; revisit nanobind for v0.2 | Accepted | 2026-04-26 | 5b | — |
| [ADR-0013](0013-bfs-table-provider-rename.md) | Reframe `SKProvider` as `BFSTableProvider` / sanity oracle | Accepted | 2026-04-26 | 2.5 | partial supersession of ADR-0004 |
| [ADR-0014](0014-failed-approach-tracking.md) | Failed-approach tracking as project policy | Accepted | 2026-04-26 | always | — |
| [ADR-0015](0015-apache-license.md) | License: Apache 2.0 with patent grant | Accepted | 2026-04-26 | always | — |
| [ADR-0016](0016-conference-target-ladder.md) | Conference target ladder: QCE26 → CGO27 → optional OOPSLA/PLDI | Accepted | 2026-04-26 | 5c | — |
| [ADR-0017](0017-reproducibility-infrastructure.md) | Reproducibility: Dockerfile + flake.nix + Zenodo DOI + papers/ | Accepted | 2026-04-26 | 2.5 | — |
| [ADR-0018](0018-mlir-stretch-optional.md) | MLIR exposure via `qfault.fto` custom dialect — Stage 7 stretch, optional | Draft | 2026-04-26 | 7 | — |
| [ADR-0019](0019-boundary-conventions.md) | Surface code boundary convention: GoSC dashed=X / solid=Z | Accepted | 2026-04-26 | 3 | — |
| [ADR-0020](0020-cnot-recipe.md) | Logical CNOT recipe: MZZ + SPLIT + MXX + X-meas + Pauli-frame correction | Accepted | 2026-04-26 | 3 | — |
| [ADR-0021](0021-stim-oracle-integration.md) | Stim oracle: FetchContent v1.15.0, `has_flow` primary, detector backstop | Accepted | 2026-04-26 | 2.5 | — |

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

If you are starting Stage 3 work, additionally:
1. ADR-0006 (routing algorithm)
2. ADR-0019 (boundary conventions)
3. ADR-0020 (CNOT recipe)
4. ADR-0021 (Stim integration)

If you are starting Stage 4 work, additionally:
- ADR-0003 (why d is global; what changes for variable-d)
- ADR-0007 (MSD factory catalog)

If you are evaluating QFault for OSS contribution or artifact submission:
- ADR-0015 (license)
- ADR-0016 (conference plan)
- ADR-0017 (reproducibility infrastructure)

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
