# Architecture Decision Records — Index

All significant architectural decisions are recorded here.
Before any session proposes a design change, read relevant ADRs.
Use `/adr` slash command to consult and create ADRs.

> **Rule:** Propose an ADR before: adding a dependency, changing a numerical
> convention, switching synthesis algorithms, or changing the IR schema.

---

## Index

| # | Title | Status | Date | Supersedes |
|---|-------|--------|------|-----------|
| [ADR-0001](0001-ir-representation.md) | IR representation: two-level vs. two-module | Accepted | 2026-04-24 | — |
| [ADR-0002](0002-synthesis-provider-interface.md) | SynthesisProvider: Concept vs. virtual base | Accepted | 2026-04-24 | — |
| [ADR-0003](0003-global-code-distance.md) | Global code distance d for v0.1; variable-d deferred | Accepted | 2026-04-24 | — |
| [ADR-0004](0004-gridsynth-as-default.md) | GridSynth as default synthesiser; SK as benchmark | Accepted | 2026-04-24 | — |
| ADR-0005 | QIR output spec version pinning | Draft | — | — |
| ADR-0006 | Routing algorithm: greedy Manhattan heuristic for v0.1 | Draft | — | — |
| ADR-0007 | MSD factory model: earliest-available scheduling | Draft | — | — |

---

## ADR Status Definitions

| Status | Meaning |
|--------|---------|
| **Draft** | Under consideration, not yet decided |
| **Accepted** | Decision made and active |
| **Deprecated** | Was accepted, now superseded |
| **Superseded** | Replaced by a later ADR (link to replacement) |
