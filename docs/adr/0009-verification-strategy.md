# ADR-0009: Verification strategy — Stim oracle + MQT QCEC, framed as validation not verification

**Status:** Accepted
**Date:** 2026-04-26
**Supersedes:** —

---

## Context

A solo C++20 compiler cannot ship Coq pass proofs in v0.1 timelines.
PLDI/POPL reviewers will rightly distinguish **verification** (formal
proof) from **validation** (test-based equivalence checking). QFault must
commit to a defensible label and stand behind it consistently across the
README, the arXiv preprint, and any conference submission.

---

## Decision

QFault ships **validation, not verification**, in v0.1–v0.2.

The validation layer comprises three components:

(a) **Stim tableau-equivalence on Clifford segments** as a CI gate.
    `stim::Circuit::has_flow(stim::Flow)` is the primary equivalence API.
    Backstop with **detector-distribution comparison**: strip noise via
    `circuit.without_noise()`, then assert
    `count_determined_measurements() == num_measurements()` and run 1024
    sweep-bit-driven shots. See ADR-0021.

(b) **MQT QCEC equivalence checking on Clifford+T blocks ≤ 8 qubits** as
    a CI gate. CI logic: demand `Equivalent` or
    `EquivalentUpToGlobalPhase` for ≤ 8 qubits; accept
    `ProbablyEquivalent` (simulation + ZX both pass without disproof) for
    > 8 qubits. Map exit codes 0/2/3/4 = pass/not-eq/probably-not-eq/
    no-info.

(c) **Golden-circuit regression tests** for representative algorithms
    (BV-10 d=5, QFT-{4,8}, small adders), committed under
    `bench/golden/<n>.stim` + `.dem` + `.dets`.

**Documentation discipline:** the README, arXiv preprint, and any
conference submission use **"validated"** rather than **"verified"**.
"Verification" is reserved for the optional Stage 7 formal-methods stretch
(ADR-0018 Option B/C).

---

## Alternatives Considered

| Alternative | Why Rejected |
|---|---|
| Coq pass proofs in v0.1 | Out of scope solo; reviewer will rightly demand mechanized proofs |
| QCEC-only (no Stim) | Misses Clifford-segment scaling; QCEC times out on large blocks |
| Stim-only (no QCEC) | Cannot check non-Clifford blocks |
| No validation layer (trust the tests) | Insufficient for OOPSLA artifact evaluation |
| Custom equivalence checker | Reviewer red flag; reuse production-grade external tools |

---

## Consequences

**Positive:**
- "Validated" is defensible against reviewer pushback.
- Stim and QCEC are both production-grade tools maintained by external
  groups, so the CI gate inherits their correctness reputation.
- Stim integration via FetchContent (target `libstim`, NOT `stim` —
  that's the CLI binary; linking will fail).
- QCEC v3.5.0 (Feb 2026) is current; FetchContent the C++ source with
  `BUILD_MQT_QCEC_BINDINGS=OFF` to avoid pulling in nanobind.

**Negative / Trade-offs:**
- QCEC scaling limits Clifford+T verification to ≤ 8 qubits in the strict
  regime; the genuine 4ⁿ-memory limit applies to *naïve unitary
  construction* (Burgholzer-Wille TCAD 2021; alternating-DD scales to
  >100 qubits in practice).
- Stim integration adds a build dependency; mitigated by FetchContent
  with explicit version pin (Stim v1.15.0 "Terror of the Tag", May 2025
  is the target).
- Stim's C++ ABI is openly documented as unstable across versions; budget
  one tag-bump cycle every 6–9 months.
- MQT QCEC moved repos (`cda-tum/mqt-qcec` → `munich-quantum-toolkit/qcec`) and dropped pybind11 for nanobind in v3.4.0. Both add real build-system complexity.

**Risks:**
- Mixing Stim SIMD widths across translation units causes ODR-violation
  crashes; always use `stim::MAX_BITWORD_WIDTH`. For golden-test
  reproducibility across machines, pin `-DSIMD_WIDTH=64` in
  `cmake/stim_config.cmake`.
- Defining a `DETECTOR` on non-deterministic measurements raises
  `InvalidGaugeException` from `detector_error_model()`; first-round
  detectors must reference only the reset state, bulk detectors must
  reference current AND previous-round measurements of the same
  stabilizer.
- For DEM extraction, must add at least one tiny noise channel; pure
  noiseless circuits return empty DEMs.

---

## Implementation Notes for AI Sessions

When loading the memory bank:
- Use the word **"validated"** not **"verified"** in all documentation.
- Stim and QCEC integration are **Stage 2.5 deliverables** (see
  `docs/phases/stage-2.5-verify-bench/spec.md`).
- Do NOT commit to formal proofs in v0.1.
- Stim umbrella header is `#include "stim.h"` (quoted; `src/` is the
  public include root). Critical sub-headers: `stim/circuit/circuit.h`,
  `stim/simulators/{tableau,frame}_simulator.h`, `stim/dem/...`.
- For each Stage 3 reference circuit, commit four files under
  `bench/golden/`: `circuit.stim` (text, deterministic dump), `circuit.dem`
  (detector error model), `reference_sample.dets` (one shot, sparse
  `dets` format), and `stim_version.txt` (exact version assertion).
- `dets` output format only; **never** `b8`/`r8` (binary, not
  diff-friendly).

---

## References

- Stim (Gidney 2021); v1.15.0 tag pinned in `cmake/stim_config.cmake`
- MQT QCEC (Burgholzer & Wille 2021); v3.5.0 tag pinned
- VOQC (Hietala et al.) — for contrast on actual mechanized verification
- ADR-0010 (output backend portfolio — Stim emission is a backend, not a
  side script)
- ADR-0017 (reproducibility infrastructure — golden file workflow)
