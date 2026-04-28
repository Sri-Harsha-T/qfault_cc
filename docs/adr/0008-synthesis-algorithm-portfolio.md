# ADR-0008: Synthesis algorithm portfolio — Ross-Selinger now, Kliuchnikov-2023 in Stage 6

**Status:** Accepted
**Date:** 2026-04-26
**Supersedes:** —

---

## Context

Qualtran ships both Ross-Selinger and Kliuchnikov-2023 mixed-fallback /
repeat-until-success synthesis. QFault v0.1 ships only Ross-Selinger (via
the `gridsynth` Haskell subprocess wrapper) and a BFS-table oracle (see
ADR-0013). Closing the algorithm-portfolio gap is the most visible
synthesis-quality differentiator versus Qualtran.

---

## Decision

QFault commits to a **two-phase synthesis algorithm portfolio**:

**Phase 1 (v0.1, Stage 2 done):**
- `GridSynthProvider` — Ross-Selinger via Haskell `gridsynth` subprocess
  (ADR-0004).
- `SKProvider` / `BFSTableProvider` — depth-7 BFS table sanity oracle
  (ADR-0013).

**Phase 2 (v0.2, Stage 6):**
- `NativeRSProvider` — native C++ Ross-Selinger using MPFR (4.2) + GMP
  (6.3) for exact arithmetic. Behind the existing `SynthesisProvider`
  Concept; zero refactoring at the call site.
- `KliuchnikovProvider` — Kliuchnikov-2023 mixed-fallback / RUS protocol
  as a second native provider.

No refactoring at the `TGateSynthesisPass<Provider>` call site is
required for Phase 2 — the Concept boundary (ADR-0002) was designed to
make this addition zero-friction.

---

## Alternatives Considered

| Alternative | Why Rejected |
|---|---|
| Add Kliuchnikov-2023 in v0.1 | Stage 2 is already done; reopening would delay Stage 3 |
| Wrap an existing Kliuchnikov implementation by subprocess | No widely-deployed open-source binary equivalent to gridsynth exists for Kliuchnikov-2023 |
| Skip Kliuchnikov entirely | Cedes Qualtran-frontier ground; weakens differentiation |
| Native C++ Ross-Selinger only (skip Kliuchnikov) | Closes the C++ throughput gap but not the algorithm-portfolio gap |

---

## Consequences

**Positive:**
- Stage 6 closes the most visible synthesis-portfolio gap.
- The Concept boundary (ADR-0002) makes the addition zero-friction.
- `NativeRSProvider` answers the "C++ throughput on synthesis" criticism
  that ADR-0004 acknowledged and explicitly retracted for v0.1.
- Stage 6 stage gate is falsifiable: ≥5× throughput over the gridsynth
  subprocess wrapper at matched T-count and ε=10⁻¹⁰ on a 10⁴ random
  rotation corpus.

**Negative / Trade-offs:**
- Stage 6 timing is contingent on user availability and on MPFR/GMP
  integration; honestly labeled as optional / contingent.
- MPFR + GMP add build dependencies; mitigated by FetchContent
  pin-by-version pattern already used for GoogleTest.

**Risks:**
- Kliuchnikov-2023 implementation complexity is non-trivial (mixed-fallback
  is more involved than Ross-Selinger); mitigated by treating it as a
  Stage 6 deliverable with its own kickoff and ADR.
- T-count parity with Qualtran's Kliuchnikov implementation must be
  validated against published reference values; budget 2–3 weeks for the
  validation harness alone.

---

## Implementation Notes for AI Sessions

When loading the memory bank:
- Kliuchnikov-2023 is a **Stage 6 deliverable**, NOT a v0.1 claim. Do
  NOT advertise it in the README, arXiv preprint, or QCE26 submission
  until Stage 6 ships.
- Native C++ Ross-Selinger is also Stage 6. Until then, the "C++
  throughput on synthesis" pitch is retracted (see ADR-0004).
- When implementing `NativeRSProvider`, pin MPFR ≥ 4.2 and GMP ≥ 6.3 via
  FetchContent or system find_package; document the build matrix in
  Stage 6's `kickoff.md`.

---

## References

- Kliuchnikov, Maslov, Mosca 2023, *Asymptotically optimal approximation
  of single qubit unitaries by Clifford and T circuits using a constant
  number of ancillary qubits* (arXiv:1212.6964 and follow-ups)
- Ross & Selinger 2016, *Optimal ancilla-free Clifford+T approximation of
  z-rotations*
- Qualtran rotation_synthesis module
- ADR-0002 (SynthesisProvider Concept — enables this portfolio)
- ADR-0004 (GridSynth as v0.1 default)
- ADR-0013 (Reframing SKProvider as fallback)
