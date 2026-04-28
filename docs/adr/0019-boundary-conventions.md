# ADR-0019: Surface code boundary convention — GoSC dashed=X / solid=Z

**Status:** Accepted
**Date:** 2026-04-26
**Supersedes:** —

## Context

Lattice surgery requires a fixed convention for which boundaries are "X-rough"
vs "Z-rough", and which patch orientation corresponds to logical X_L and Z_L.
Different sources use mutually inconsistent conventions (Horsman 2012, Litinski
2018/2019 "GoSC", Fowler-Gidney 2018, Chamberland-Campbell 2022). Picking one
and enforcing it in the IR avoids a class of subtle bugs where merges produce
the wrong logical operator.

## Decision

QFault adopts the **GoSC convention** (Litinski 2019, *A Game of Surface Codes*):

1. **Dashed boundaries are X-rough**, **solid boundaries are Z-rough**.
2. **Default patch orientation:** Z_L vertical, X_L horizontal.
3. **Merging rules:**
   - Merging two patches along facing **Z-edges** yields **MZZ** (Z⊗Z parity).
   - Merging two patches along facing **X-edges** yields **MXX** (X⊗X parity).
   - **MYY without twist defects is forbidden in v0.1**; would require patch-deformation
     gymnastics. A `--use-twists` flag for LSwT mode is reserved for v0.2+ if the
     hardware target supports weight-5 stabilizers.
4. **Routing-channel matching:** an A* router must match X-edge to X-edge or
   Z-edge to Z-edge. Mixing is a logic error — the IR's `MergeOp::validate()`
   asserts at construction.
5. **Boundary type is part of `NodeId` composition** in the routing graph
   (`(tile, side)` tuples), so structural matching falls out of A* naturally.

## Alternatives Considered

| Alternative | Why Rejected |
|---|---|
| Horsman 2012 convention (dashed=Z, solid=X) | Inverse of GoSC — the more recent literature is GoSC-aligned |
| No global convention; encode per-patch | Forces every pass to track which convention each patch uses; bug-prone |
| Allow MYY in v0.1 via patch deformation | Implementation cost (twist defects, weight-5 stabilizers) too large for v0.1 |
| Z_L horizontal, X_L vertical | Symmetric choice; GoSC vertical-Z is more common in current papers |

## Consequences

**Positive:** Single convention, mechanically enforced by IR validation. A* router
naturally rejects illegal merges because of NodeId structure. Surface-code
literature reading is easier — all citations in QFault use the same convention.

**Negative / Trade-offs:** Patches imported from external tools (e.g. liblsqecc)
may need orientation flips. Documented in the import path.

**Risks:** A future "use-twists" mode might want to relax the no-MYY rule; the
ADR explicitly reserves this for v0.2+ with a follow-up ADR.

## Implementation Notes for AI Sessions

When loading the memory bank:

- **MERGE pairs matching boundaries only** (X↔X or Z↔Z). Mixing throws at IR construction.
- **MeasBasis has only X and Z**, never Y (already enforced; documented in `.claude/rules/qec.md`).
- Routing NodeId encodes `(tile, side)` so the A* heap naturally rejects
  illegal merges. Do not "hide" boundary type by encoding only the tile.
- Patch orientation in `PatchCoord` defaults to `{Z_vertical: true}`; flipping
  this is allowed only at patch construction, never during routing.

## References

- Litinski 2019, *A Game of Surface Codes*, arXiv:1808.02892, §2.1
- Horsman et al. 2012, "Surface code quantum computing by lattice surgery"
- Fowler & Gidney 2018, *Low overhead quantum computation using lattice surgery*, arXiv:1808.06709