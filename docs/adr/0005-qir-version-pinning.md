# ADR-0005: QIR output spec version pinning

**Status:** Accepted
**Date:** 2026-04-26
**Supersedes:** —

---

## Context

QFault's Stage 5a backend portfolio includes Quantum Intermediate
Representation (QIR). The QIR specification has multiple versions and
stewards (Microsoft's original drop, the QIR Alliance LLVM-based spec).
Version churn risks coupling QFault releases to upstream spec instability,
and OOPSLA-style artifact evaluation will reject non-reproducible builds.

---

## Decision

QFault pins **QIR Alliance v0.1 base profile** for v0.1 release and treats
the QIR backend as a swappable module behind a `QIRBackend` interface. The
pinned version is recorded in `cmake/qir_version.cmake` and propagated as a
compile definition `QFAULT_QIR_SPEC_VERSION="0.1"`. Any spec version bump
requires a follow-up ADR with a measured-change rationale.

---

## Alternatives Considered

| Alternative | Why Rejected |
|---|---|
| Track QIR Alliance HEAD | Spec churn would force frequent QFault re-releases |
| Microsoft QIR original | Less actively maintained; smaller user base |
| Do not pin (latest at build time) | Non-reproducible builds; OOPSLA artifact evaluation will reject |
| Multiple version backends in v0.1 | Multiplies test surface; defer until ≥2 downstream consumers request it |

---

## Consequences

**Positive:**
- Reproducible QIR output across CI runs and release tags.
- Backend swap is one CMake variable change.
- Aligns with ADR-0017 reproducibility infrastructure (Dockerfile +
  flake.nix).

**Negative / Trade-offs:**
- QFault must explicitly bump the pinned version in a new ADR when
  downstream consumers (Microsoft Quantum, Q#, Azure Quantum) move; the
  lag is acceptable for a research compiler.

**Risks:**
- If the QIR Alliance pivots to a fundamentally different IR shape, the
  swappable backend may need a deeper refactor; mitigated by keeping QIR
  emission isolated from the IR layer (no `QFaultIR` types depend on QIR
  identifiers).

---

## Implementation Notes for AI Sessions

When loading the memory bank: QIR version is in `cmake/qir_version.cmake`.
Do NOT bump without a new ADR. The `QIRBackend` interface lives in
`include/qfault/backend/QIRBackend.hpp` (Stage 5a deliverable). The pinned
spec version must be propagated to runtime as a compile definition so that
`qfault_tests` can assert against it.

---

## References

- QIR Alliance specification (LLVM-based, v0.1 base profile):
  https://github.com/qir-alliance/qir-spec
- Microsoft QIR original drop
- ADR-0010 (output backend portfolio)
- ADR-0017 (reproducibility infrastructure)
