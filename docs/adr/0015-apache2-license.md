# ADR-0015: License — Apache 2.0

**Status:** Accepted
**Date:** 2026-04-26
**Supersedes:** —

---

## Context

License choice affects downstream adoption, patent risk, and reviewer
perception. `memory-bank/projectbrief.md` already references Apache 2.0;
this ADR records the decision and rejected alternatives explicitly so
that future contributors do not re-open the question.

---

## Decision

QFault is released under **Apache License, Version 2.0** with explicit
patent grant, mirroring Qiskit, Stim, CUDA-Q, PennyLane, and TKET.

Every source file under `src/`, `include/`, `tests/`, and `bindings/`
carries the SPDX header:

```
// SPDX-License-Identifier: Apache-2.0
// Copyright 2026 The QFault Authors
```

The canonical license text lives at `LICENSE` in the repo root. A
`NOTICE` file lists external dependencies and their licenses.

External dependencies and their compatibility:
- GoogleTest (BSD-3-Clause): compatible.
- Google Benchmark (Apache-2.0): compatible.
- pybind11 (BSD-style): compatible.
- Stim (Apache-2.0): compatible.
- MQT QCEC (MIT): compatible.
- gridsynth (BSD-3-Clause): compatible.
- liblsqecc (GPL-3.0): **incompatible** — do NOT vendor; reference only,
  via subprocess wrapper if needed for benchmarking.

---

## Alternatives Considered

| Alternative | Why Rejected |
|---|---|
| MIT | No patent grant; unsuitable for a project touching quantum-architecture IP |
| GPL-3 | Strong copyleft kills downstream commercial integration; incompatible with Qiskit/Stim ecosystem norms |
| AGPL | Even stronger copyleft; unsuitable for library use |
| BSD-3-Clause | No patent grant |
| Mozilla Public License | File-level copyleft adds complexity without benefit for this audience |

---

## Consequences

**Positive:**
- Drop-in compatible with the open-source quantum compiler ecosystem;
  no friction for downstream users.
- Explicit patent grant protects QFault contributors and downstream
  consumers from quantum-architecture patent risk.
- SPDX-headered files are mechanically scannable by automated license
  checkers (REUSE compliance via `reuse lint`).

**Negative / Trade-offs:**
- None substantive for a research project.

**Risks:**
- liblsqecc incompatibility means QFault cannot vendor liblsqecc code
  for performance comparisons; must use subprocess invocation or
  re-implement the comparison logic. This is documented in ADR-0006 and
  Stage 3's `kickoff.md`.

---

## Implementation Notes for AI Sessions

When loading the memory bank:
- Every new source file gets the SPDX header. The `format` CMake target
  should be extended to verify SPDX presence (Stage 5c task).
- The `LICENSE` file at repo root is the canonical Apache 2.0 text.
- The `NOTICE` file lists external dependencies; update when adding
  any new FetchContent dependency.
- Do NOT vendor GPL-3.0 code (liblsqecc) into the QFault tree. Reference
  via subprocess only.

---

## References

- Apache License, Version 2.0:
  https://www.apache.org/licenses/LICENSE-2.0
- SPDX License List: https://spdx.org/licenses/
- REUSE Software (license compliance): https://reuse.software/
- Qiskit, Stim, CUDA-Q, PennyLane, TKET licensing for ecosystem precedent
- liblsqecc license (GPL-3.0): documented incompatibility
