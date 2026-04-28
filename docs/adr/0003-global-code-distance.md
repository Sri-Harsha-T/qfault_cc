# ADR-0003: Global code distance d for v0.1; variable-d deferred

**Status:** Accepted
**Date:** 2026-04-25 (post-implementation findings 2026-04-26)
**Supersedes:** —

---

## Context

Real surface-code architectures use **variable** code distance: data qubits
at high d, ancilla regions at lower d, factory output at higher d than the
consumer. Variable-d is a real Stage 4 optimization (Beverland 2022 Appendix C).
The question is whether v0.1 should support it.

---

## Decision

v0.1 fixes a single global `codeDistance` validated in `PassContext` (must be
**odd, ≥ 3**, asserted in the constructor). Variable-d is deferred to v0.2 or
later, gated on a follow-up ADR extending the `PassContext` distance surface.

---

## Alternatives Considered

| Alternative | Why Rejected |
|---|---|
| Per-region `std::map<RegionId, int>` distances | Premature; no Stage 1–2 pass consumes region-keyed distance |
| Per-patch distance on `PatchOp` | Couples IR layout to optimization decisions belonging to Stage 4 |
| Distance as a global compile-time template parameter | Forces recompilation per circuit; eliminates runtime configurability |

---

## Consequences

**Positive:**
- Simpler `PassContext` invariants (one number to validate).
- Odd-and-≥3 check rules out invalid distances at construction time —
  wrong distances cannot exist as values.
- Through Stage 1+2 (211/211 tests), no pass has needed variable d.

**Negative / Trade-offs:**
- Stage 4 MSD scheduling will need variable d for realistic factory cost
  numbers — high-d data and lower-d ancilla regions is a known optimization
  in Beverland 2022. Stage 4 will require a follow-up ADR extending the
  `PassContext` distance surface.
- Resource estimates produced under the global-d assumption must be flagged
  as **upper bounds**, not optimal — the arXiv preprint and any
  `ResourceReport` JSON output must include `"global_d_assumption": true`.

**Risks:**
- If Stage 4 attempts to retrofit variable-d into a `PassContext` that has
  stabilized, downstream passes may need re-validation. Mitigation: keep
  the global-d API and add an opt-in region-keyed extension rather than
  mutating the existing surface.

**Empirical finding (post-implementation, 2026-04-26):**
The constructor validation (`codeDistance % 2 == 1 && codeDistance >= 3`)
caught two test misconfigurations during Stage 1 development. The global-d
assumption did not block any Stage 1 or Stage 2 deliverable.

---

## Implementation Notes for AI Sessions

When loading the memory bank: `PassContext::codeDistance()` is a single
`unsigned` through Stage 3. Do NOT add region-keyed distance until Stage 4
kickoff. The validator is in `PassContext`'s constructor — preserve the
odd-and-≥3 invariant. All passes read `d` from `PassContext`; never
hardcode.

---

## References

- `include/qfault/passes/PassContext.hpp`
- Beverland 2022 Appendix C (variable-d cost models)
- ADR-0007 (MSD factory selection — depends on this distance surface)
