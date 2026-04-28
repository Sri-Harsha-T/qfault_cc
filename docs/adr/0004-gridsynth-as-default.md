# ADR-0004: GridSynth as default; BFS-table oracle as benchmark baseline

**Status:** Accepted (with limitation; superseded in part by ADR-0013)
**Date:** 2026-04-25 (post-implementation findings 2026-04-26)
**Supersedes:** —

---

## Context

QFault must ship a default T-gate synthesizer for v0.1 and a fallback when
the default is unavailable. The default should be the field's reference
algorithm; the fallback should be self-contained and require no external
dependencies.

---

## Decision

`GridSynthProvider` (subprocess wrapper around the Haskell `gridsynth`
binary, Ross-Selinger 2016) is the default. `SKProvider` (depth-7 BFS over
the generator set `{H, T, Tdg, S, Sdg, Z}` with trivial-cancellation
pruning, capped at 512 entries, closest-match by Frobenius distance modulo
global phase) is the always-available benchmark baseline. CMake's
`find_program` detects gridsynth at configure time and sets
`QFAULT_HAS_GRIDSYNTH`; absence emits a configure-time `WARNING` and tests
that depend on the binary use `GTEST_SKIP()`.

---

## Alternatives Considered

| Alternative | Why Rejected |
|---|---|
| Native C++ Ross-Selinger as default in v0.1 | 8–12 weeks of additional Stage 2 work; deferred to Stage 6 (see ADR-0008) |
| Real Solovay-Kitaev recursion as fallback | Implementation cost and lack of pre-existing C++ reference; BFS-table oracle is sufficient as a sanity baseline |
| No fallback (hard-fail without gridsynth) | Breaks `quick-test.sh < 60s` workflow on machines without Haskell |

---

## Consequences

**Positive:**
- `GridSynthProvider` reuses the field-reference Ross-Selinger
  implementation, so T-count outputs match newsynth/rsgridsynth at
  matched ε.
- `SKProvider` makes Stage 2 testable on any machine without Haskell
  (CI green without gridsynth installed).

**Negative / Trade-offs (CRITICAL CLARIFICATION — added 2026-04-26):**

`SKProvider` is **NOT** real Solovay-Kitaev. It is a depth-7 BFS over a
512-entry table with achievable accuracy bounded around `1e-3` Frobenius
distance — **four orders of magnitude looser** than `PassContext`'s default
`synthesisEpsilon = 1e-10`. The `eps` argument is essentially ignored
beyond serving as a downstream tolerance check. The name and module path
imply more than the implementation delivers, and
`tests/unit/test_SKProvider.cpp` uses a `1e-3` tolerance to accommodate
this. **This limitation was not surfaced in the original version of this
ADR and must be addressed.**

Additionally, `GridSynthProvider` is a `popen` subprocess wrapper, so
synthesis throughput is bottlenecked by Haskell startup + pipe IO per
call. The "C++ throughput" pitch applies to `PassManager`, IR traversal,
and downstream stages — **not to synthesis itself in v0.1**.

**Risks:**
- External readers may interpret "SKProvider" as a real Solovay-Kitaev
  implementation; a benchmarker comparing QFault's "SK" against another
  tool's real SK will see misleading results.
- **Mitigation:** ADR-0013 reframes `SKProvider` as a fallback / sanity
  oracle, recommends renaming to `BFSTableProvider` in v0.2 (preserving
  `SKProvider` as a deprecated alias for one release), and tightens the
  README accordingly.

**Empirical finding (post-implementation, 2026-04-26):**
The "no T/Tdg remain after `TGateSynthesisPass<SKProvider>`" assertion
documented in CHANGELOG.md "Failed Approaches" was a direct consequence of
this misframing — for `θ = π/4`, the closest table entry IS `T`, so the
oracle correctly returns `{T}` and T gates remain. The integration test
now uses a `CliffordOnlyProvider` mock to verify the pass mechanism.

---

## Implementation Notes for AI Sessions

When loading the memory bank:
- `GridSynthProvider` is a subprocess wrapper, **never** a native
  implementation.
- `SKProvider` is a BFS table, **never** real SK. From v0.2 onward it is
  renamed `BFSTableProvider` (see ADR-0013).
- Do NOT claim "C++ throughput on synthesis" in v0.1 documentation.
- The fix path is Stage 6 (ADR-0008), not Stage 2.

---

## References

- `src/qfault/passes/synthesis/GridSynthProvider.cpp`
- `src/qfault/passes/synthesis/SKProvider.cpp`
- `tests/unit/test_SKProvider.cpp`
- ADR-0008 (Synthesis algorithm portfolio — Stage 6 deliverables)
- ADR-0013 (Reframing SKProvider as a fallback / sanity oracle)
- Ross & Selinger 2016, *Optimal ancilla-free Clifford+T approximation of
  z-rotations*
- newsynth tool: https://github.com/kenmcken/newsynth
