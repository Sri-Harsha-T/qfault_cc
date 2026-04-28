# ADR-0013: Reframing SKProvider as a fallback / sanity oracle

**Status:** Accepted
**Date:** 2026-04-26
**Supersedes:** ADR-0004 (in part — limitation note and naming)

---

## Context

`SKProvider` is named and located as if it were a Solovay-Kitaev
implementation, but the algorithm is a depth-7 BFS over a 512-entry
generator table with achievable accuracy bounded around `1e-3` Frobenius
distance, **four orders of magnitude looser** than `PassContext`'s default
`synthesisEpsilon = 1e-10`. The mismatch between name and implementation
is a documentation bug that will mislead external readers.

The team's own `CHANGELOG.md` "Failed Approaches" log already records that
an over-strict assertion ("no T remain after `TGateSynthesisPass<SKProvider>`")
was based on this same misreading — for `θ = π/4`, the closest table entry
IS `T`, so the oracle correctly returns `{T}` and T gates remain. This is
not a bug in the oracle; it is a naming/documentation problem.

---

## Decision

(a) **Rename** the public symbol from `SKProvider` to `BFSTableProvider`
    in v0.2 (preserving `SKProvider` as a deprecated alias for one
    release):

    ```cpp
    // include/qfault/passes/synthesis/BFSTableProvider.hpp
    class BFSTableProvider {
        // existing implementation
    };

    // include/qfault/passes/synthesis/SKProvider.hpp (deprecated alias)
    [[deprecated("Renamed to BFSTableProvider in v0.2; "
                 "use BFSTableProvider for new code")]]
    using SKProvider = BFSTableProvider;
    ```

(b) **Add a doc-comment** on the header declaring the algorithm, the
    table size, the achievable accuracy (≈1e-3), and the explicit
    instruction *"Use as a benchmark baseline / sanity oracle only; do
    not use as a production synthesizer."*

(c) **Update the README and ADR-0004** to reflect the limitation.

(d) **Keep the `SynthesisProvider` Concept satisfaction** —
    `BFSTableProvider` remains a valid provider for testing and
    benchmarking.

---

## Alternatives Considered

| Alternative | Why Rejected |
|---|---|
| Implement real SK recursion in v0.1 | 4–8 weeks of additional Stage 2 work; deferred to Stage 6 (ADR-0008) |
| Remove `SKProvider` entirely | Loses a useful sanity oracle for testing without gridsynth |
| Document the limitation but keep the name | Externally misleading; reviewers will flag |
| Tighten the BFS to 1e-10 by deepening | Combinatorial explosion; the BFS approach cannot hit 1e-10 without explosive table growth |

---

## Consequences

**Positive:**
- External readers see the truth in the symbol name.
- The "no T remain" failed-approach is recontextualized as **correct
  behavior** of a benchmark oracle, not a bug.
- Stage 6's `NativeRSProvider` and `KliuchnikovProvider` (ADR-0008) get
  unambiguous names alongside; no naming collision.

**Negative / Trade-offs:**
- Renaming touches `tests/unit/test_SKProvider.cpp`,
  `test_TGateSynthesisPass.cpp`, and any external code already using the
  symbol; mitigated by keeping `SKProvider` as a deprecated alias for
  v0.2.

**Risks:**
- None substantive. The rename is a v0.2 task and external adopters can
  migrate at their own pace within the v0.2 release window.

---

## Implementation Notes for AI Sessions

When loading the memory bank:
- The BFS-table provider is named `BFSTableProvider` from v0.2 onward.
  `SKProvider` is a deprecated alias.
- Achievable accuracy ≈ 1e-3.
- Default `synthesisEpsilon` is 1e-10; `BFSTableProvider` ignores `eps`
  beyond using it as a tolerance check.
- For production synthesis, use `GridSynthProvider` (v0.1) or the native
  Stage 6 providers `NativeRSProvider` / `KliuchnikovProvider`.
- v0.1 commits should keep the existing `SKProvider` filename; rename
  happens in the v0.2 milestone with a single commit that creates the
  alias and adds the deprecation attribute.

---

## References

- `src/qfault/passes/synthesis/SKProvider.cpp` (current implementation)
- ADR-0004 (the original GridSynth / SKProvider decision; this ADR
  supersedes the limitation note)
- ADR-0008 (Stage 6 native synthesis — replaces the production role
  `SKProvider` was incorrectly thought to fill)
- `CHANGELOG.md` "Failed Approaches" entry on the no-T-remain assertion
