# ADR-0004: GridSynth as Default Synthesiser; SK as Benchmark Baseline

**Status:** Accepted  
**Date:** 2026-04-24  
**Supersedes:** —

---

## Context

QFault's SynthesisPass needs a default algorithm. The two candidates are:
- **Solovay-Kitaev (SK):** Classic, well-known, implementable in pure C++
- **GridSynth:** Number-theoretic, state-of-the-art, nearly T-count optimal

---

## Decision

**GridSynth is the default `SynthesisProvider`.** SK is implemented solely as
a benchmark baseline for comparison. SK must NOT be the default in any
user-facing configuration, documentation, or README example.

---

## Rationale

SK produces gate sequences of length O(log^{3.97}(1/ε)), which is significantly
longer in T-count than GridSynth's near-optimal sequences. Since every T-gate
costs one MSD factory cycle, SK's T-count overhead directly translates to longer
circuit times and larger physical qubit footprints. Domain experts will immediately
reject QFault as non-state-of-the-art if SK is the default.

---

## Consequences

**Positive:** QFault produces competitive T-counts from day one.

**Negative:** GridSynth is an external dependency (Haskell binary or C++ port).
This complicates builds and packaging. Mitigation: provide a CMake option
`-DQFAULT_GRIDSYNTH_PATH` and a fallback to SK with a runtime warning.

---

## Implementation Notes for AI Sessions

- In README and docs, ALL examples use GridSynth
- SK is documented as "benchmark baseline" — never as a usable alternative
- If GridSynth is not found at build time, CMake should print:
  `WARNING: GridSynth not found. Falling back to Solovay-Kitaev (not T-optimal).`
- Benchmark comparison (SK vs GridSynth T-count) belongs in `docs/benchmarks.md`
  and the arXiv preprint — NOT in the main API docs
- T-count table to validate against: Ross & Selinger 2016, Table 1

---

## References

- Ross & Selinger, "Optimal ancilla-free Clifford+T approximation" (2016)
- Dawson & Nielsen, "The Solovay-Kitaev algorithm" (2005)
- newsynth tool: https://github.com/kenmcken/newsynth
