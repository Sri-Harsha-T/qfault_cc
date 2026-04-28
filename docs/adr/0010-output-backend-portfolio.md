# ADR-0010: Output backend portfolio — QASM 3.0 + QIR + Stim native

**Status:** Accepted
**Date:** 2026-04-26
**Supersedes:** —

---

## Context

A research FT compiler needs at least one backend per audience: QASM 3.0
for the broad ecosystem, QIR for Microsoft / Azure-aligned tooling, and
Stim native for verification. Reviewers from each community will look
first for "their" backend.

---

## Decision

v0.1 ships **three backends behind a uniform `Backend` interface**:

1. **QASM 3.0** — round-trip compatible with the existing frontend
   lexer/parser. Stage 5a deliverable.
2. **QIR** — pinned per ADR-0005 (QIR Alliance v0.1 base profile).
   Stage 5a deliverable.
3. **Stim native** — the verification enabler, NOT a side feature.
   Stage 5a deliverable, doubles as the validation oracle (ADR-0009).
   (consumes Stage 3 PHYSICAL IR directly, supports `verify/`-side equivalence).

Each backend is opt-in via a CMake flag:
- `-DQFAULT_BACKEND_QASM3=ON` (default)
- `-DQFAULT_BACKEND_QIR=ON` (default)
- `-DQFAULT_BACKEND_STIM=ON` (requires Stim FetchContent)

The interface:

```cpp
class Backend {
public:
    virtual ~Backend() = default;
    [[nodiscard]] virtual std::string_view name() const = 0;
    virtual void emit(const QFaultIRModule& module, std::ostream& out) = 0;
};
```

---

## Alternatives Considered

| Alternative | Why Rejected |
|---|---|
| QASM 3.0 only | Misses QIR audience; misses verification path |
| QIR only | QASM is the lingua franca of the broader research community |
| Stim emission as an internal-only debug feature | Underplays the verification story; raising it to a backend is the right framing |
| Add OpenQASM 2.0 backend | Deprecated; not worth the test surface |
| Backends as Concept (parallel to SynthesisProvider) | Backends are typically called once per circuit, not once per gate; virtual dispatch overhead is negligible at this granularity |

---

## Consequences

**Positive:**
- Three backends cover the three reviewer audiences (open-source
  research, Azure/Microsoft, verification community).
- Stim emission doubles as the validation oracle (ADR-0009) — write
  once, use twice.
- The QASM 3.0 backend completes the frontend ↔ backend round-trip that
  Stage 1's `tests/integration/test_qasm_roundtrip.cpp` currently checks
  via instruction-vector equality only.

**Negative / Trade-offs:**
- Three backends triple the round-trip test surface; mitigated by shared
  per-IR-instruction emission helpers in
  `src/qfault/backend/EmissionHelpers.cpp`.

**Risks:**
- Backend drift if the IR evolves; mitigated by integration tests
  (`tests/integration/test_qasm_roundtrip.cpp` already exists; QIR and
  Stim equivalents are required Stage 5a deliverables).
- Stim native backend output must be deterministic (no addresses, no
  timestamps, sorted iterables); golden-file regression catches drift.

---

## Implementation Notes for AI Sessions

When loading the memory bank:
- The `Backend` interface lives in `include/qfault/backend/Backend.hpp`.
- Stim emission is a backend, not a side script — do NOT scatter Stim
  output logic across `tests/` or `scripts/`.
- QIR version is per ADR-0005; do NOT bump without a new ADR.
- Each backend has a corresponding `tests/integration/test_<name>_roundtrip.cpp`.
- Stable serialization: no timestamps, no addresses, no hash values, sort
  all unordered iterables before output, LF line endings only,
  locale-independent numeric formatting (`std::format("{:.17g}", x)` —
  not `printf`).

---

## References

- OpenQASM 3.0 specification
- QIR Alliance v0.1 base profile
- Stim circuit format
- ADR-0005 (QIR version pinning)
- ADR-0009 (verification strategy — Stim native backend feeds the
  validation oracle)
- ADR-0017 (reproducibility infrastructure — deterministic emission)
