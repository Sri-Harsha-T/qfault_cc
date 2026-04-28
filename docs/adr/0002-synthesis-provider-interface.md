# ADR-0002: SynthesisProvider as a C++20 Concept

**Status:** Accepted
**Date:** 2026-04-25 (post-implementation findings 2026-04-26)
**Supersedes:** —

---

## Context

Synthesis providers (gridsynth wrapper, BFS-table oracle, future native
Ross-Selinger, future Kliuchnikov-2023) must be interchangeable at the
`TGateSynthesisPass` call site without virtual-dispatch overhead in inner
loops. The team must decide between a virtual interface, a `std::function`
callback, and a C++20 Concept.

---

## Decision

`SynthesisProvider` is a C++20 Concept requiring
`synthesise(angle, eps) -> std::vector<GateKind>` and
`name() -> std::string_view`. `TGateSynthesisPass<Provider>` is a class
template inheriting `PassBase`, so virtual dispatch occurs **once per pass
invocation**, not once per gate. Concept satisfaction is verified by
`static_assert` in `tests/unit/test_SynthesisProvider.cpp`.

---

## Alternatives Considered

| Alternative | Why Rejected |
|---|---|
| `class ISynthesisProvider` virtual base | One virtual call per T-gate in inner loop; defeats the "C++ performance" argument for the pipeline |
| `std::function<std::vector<GateKind>(double, double)>` | Type-erasure overhead similar to virtual; loses `name()` and any future per-provider configuration surface |
| Non-template `TGateSynthesisPass` taking `SynthesisProvider*` | Same virtual-dispatch problem at the boundary |

---

## Consequences

**Positive:**
- Zero overhead at the Concept boundary, confirmed by six `static_assert`s
  in `tests/unit/test_SynthesisProvider.cpp` for `MockProvider`, `SKProvider`,
  `GridSynthProvider` (positive cases) and `NotAProvider`, `MissingName`,
  `MissingSynthesize`, `int` (negative cases).
- Provider swap is a one-line template-argument change.
- The composition question — *can a Concept-templated component participate
  in a virtual class hierarchy?* — is answered affirmatively: virtual at the
  pass granularity, Concept at the provider granularity.

**Negative / Trade-offs:**
- Compile times grow with the number of passes that template on providers;
  mitigated by the small number of synthesis providers expected (≤4 through
  Stage 6).
- Error messages for Concept failures remain verbose under gcc-13 and
  clang-18; mitigated by the explicit `static_assert` tests that produce
  readable diagnostics.

**Risks:**
- If a future provider needs runtime selection (e.g. user-configurable at the
  CLI), an additional thin `std::variant<GridSynthProvider, NativeRSProvider,
  KliuchnikovProvider>` wrapper will be needed; this is a known future design
  point, not a current blocker.

**Empirical finding (post-implementation, 2026-04-26):**
The Concept boundary held cleanly through Stage 2 implementation
(118/118 tests). No virtual-dispatch fallback was needed. The negative
`static_assert`s caught two iterations of `MockProvider` definitions that
omitted required methods, validating the design's diagnostic value.

---

## Implementation Notes for AI Sessions

When loading the memory bank: `SynthesisProvider` is a Concept, not a class.
To add a provider, write a struct with the two required methods and add a
`static_assert(SynthesisProvider<NewProvider>)` to
`tests/unit/test_SynthesisProvider.cpp`. Do NOT add a virtual base; do NOT
edit `TGateSynthesisPass` to take a pointer.

---

## References

- `include/qfault/passes/synthesis/SynthesisProvider.hpp`
- `tests/unit/test_SynthesisProvider.cpp`
- C++20 [concept] specification
- Ross & Selinger, "Optimal ancilla-free Clifford+T approximation of
  z-rotations" (2016) — for the GridSynth output format
