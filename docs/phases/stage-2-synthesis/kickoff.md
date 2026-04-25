# Stage 2 Kickoff — Synthesis Pass (T-Gate)

**Stage:** 2 of 5
**Milestone:** "Stage 2: Synthesis Pass"
**Prerequisite:** Stage 1 complete ✅

---

## Objectives

Replace the abstract T/Tdg gates in a LOGICAL-level `QFaultIRModule` with
concrete Clifford+T sequences. The key product is `TGateSynthesisPass<Provider>`,
a generic pass parameterised by a `SynthesisProvider` Concept.

**What Stage 2 is not:**
- Not mapping logical gates to physical patches (Stage 3 — LatticeSurgeryPass)
- Not scheduling magic state factories (Stage 4)
- Not emitting output files (Stage 5)

---

## Key Design Decisions (from ADRs)

**ADR-0002 (SynthesisProvider Concept):** The synthesis interface is a C++20 Concept,
not a virtual base class. This avoids virtual dispatch overhead in the synthesis
inner loop and enables static type checking at the call site.

```cpp
template<typename T>
concept SynthesisProvider = requires(T p, double angle, double eps) {
    { p.synthesise(angle, eps) } -> std::convertible_to<std::vector<GateKind>>;
    { p.name() }                -> std::convertible_to<std::string_view>;
};
```

**ADR-0004 (GridSynth default):** GridSynth is the primary provider. Solovay-Kitaev
is implemented only as a benchmark baseline — it must not be used as the default
in production code.

**ADR-0003 (Global d):** Code distance is set globally in `PassContext`. The
synthesis pass reads `ctx.synthesisEpsilon()` for ε, never hardcoding 1e-10.

---

## GridSynth Integration Approach

GridSynth is a Haskell binary. Integration options ranked by complexity:

| Approach | Pros | Cons |
|----------|------|------|
| Subprocess call (`popen`) | Simple; no Haskell FFI | CI must install gridsynth; latency per call |
| Pre-computed lookup table | Zero latency; no binary needed | Only covers exact angles |
| Pure C++ re-implementation | No dependency | High effort; correctness risk |

**v0.1 decision:** Subprocess call with graceful fallback. If `gridsynth` binary is
not found, `GridSynthProvider::synthesise()` returns an empty vector and logs a
`DiagLevel::Warn` diagnostic. Tests that need GridSynth use `GTEST_SKIP()`.

---

## Source Layout

```
include/qfault/passes/synthesis/
  SynthesisProvider.hpp
  GridSynthProvider.hpp
  SKProvider.hpp
  TGateSynthesisPass.hpp

src/qfault/passes/synthesis/
  GridSynthProvider.cpp
  SKProvider.cpp
```

---

## Execution Order

```
Finalise ADRs 0002/0003/0004
    │
    ▼
2-A-1 SynthesisProvider Concept
    │
    ├──► 2-A-2 GridSynthProvider ──► 2-C-1/2 CMake + skip logic
    └──► 2-A-3 SKProvider
                │
                ▼
           2-A-4 Unit tests
                │
                ▼
           2-B-1 TGateSynthesisPass<Provider>
                │
                ▼
           2-B-2 Integration test (T-gate replacement)
                │
                ▼
           2-B-3 T-count validation
                │
                ▼
           2-C-3 Benchmark (≤5% overhead)
                │
                ▼
           Stage Gate
```
