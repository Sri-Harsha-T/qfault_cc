# ADR-0002: SynthesisProvider Interface — C++20 Concepts, Not Virtual Base

**Status:** Accepted  
**Date:** 2026-04-24  
**Supersedes:** —

---

## Context

`SynthesisPass` needs a pluggable interface for gate synthesis algorithms
(initially: GridSynth and Solovay-Kitaev, future: GridSynth-ε variants).
The interface must be swappable without modifying `SynthesisPass` internals.

The two natural options in C++ are:
1. Abstract base class with `virtual synthesize(...)` method
2. C++20 Concept constraining a template parameter

---

## Decision

Use a **C++20 Concept** (`SynthesisProvider`) rather than a virtual base class.
`SynthesisPass` is templated on its provider type.

```cpp
template<typename T>
concept SynthesisProvider = requires(T s, UnitaryMatrix u, double epsilon) {
    { s.synthesize(u, epsilon) } -> std::same_as<CliffordTPlusList>;
    { s.tCount(u, epsilon) }     -> std::same_as<std::size_t>;
    { s.name() }                 -> std::convertible_to<std::string_view>;
};

template<SynthesisProvider ProviderT>
class SynthesisPass : public PassBase {
    ProviderT provider_;
    // ...
};
```

---

## Alternatives Considered

| Alternative | Why Rejected |
|-------------|-------------|
| Virtual base class `ISynthesisProvider` | vtable call per gate is acceptable but C++20 Concepts give better error messages and zero overhead; also avoids the inheritance trap (no accidental slicing, no shared_ptr overhead per-gate) |
| `std::function<CliffordTPlusList(UnitaryMatrix)>` | Type-erases too much; can't query `tCount` cheaply; harder to compose providers |

---

## Consequences

**Positive:**
- Zero runtime overhead (no vtable, no heap allocation per call)
- Compiler errors at concept constraint violation are clear and localised
- New providers can be added in separate files without touching `SynthesisPass`
- Enables compile-time static asserts on provider properties

**Negative / Trade-offs:**
- `SynthesisPass` is a class template — slightly more complex build dependency graph
- Can't store heterogeneous providers in a single `vector<PassBase*>` without
  type erasure (use a factory function if runtime switching is needed)

---

## Implementation Notes for AI Sessions

- Define the Concept in `include/qfault/passes/synthesis/SynthesisProvider.hpp`
- `GridSynthProvider` wraps GridSynth binary/lib — use `subprocess` or lib call
- `SKProvider` is a pure C++ implementation — the Solovay-Kitaev recursion
- When testing providers: validate T-count against published GridSynth tables
  for standard gates (T, Tdg, Rx(π/4), etc.)
- Do NOT make `SynthesisPass` non-template to avoid the Concept — that defeats
  the purpose and reintroduces virtual dispatch
- The Concept check (`static_assert(SynthesisProvider<GridSynthProvider>)`) should
  be in a test file to catch API drift early

---

## References

- Ross & Selinger, "Optimal ancilla-free Clifford+T approximation of z-rotations" (2016) — GridSynth
- Dawson & Nielsen, "The Solovay-Kitaev algorithm" (2005)
- C++ Concepts: https://en.cppreference.com/w/cpp/language/constraints
