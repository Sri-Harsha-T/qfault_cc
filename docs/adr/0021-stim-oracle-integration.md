# ADR-0021: Stim oracle integration — FetchContent v1.15.0, has_flow primary, detector backstop

**Status:** Accepted
**Date:** 2026-04-26
**Supersedes:** —

## Context

Stage 2.5's verification harness (ADR-0009) requires Stim as a C++ library, not
a subprocess. Stim has well-known characteristics that constrain the integration:
**its C++ ABI is openly unstable across versions** (the project explicitly warns
about this), the library target is **`libstim`** (already prefixed; linking
`stim` fails because that's the CLI binary), and the SIMD-width template
parameter `W` must match across all translation units or ODR violations cause
runtime crashes.

The Stage 3 implementation research surfaced that v1.15.0 ("Terror of the Tag",
May 2025) is the latest tag as of April 2026. v1.16-dev builds exist on PyPI
but no formal release.

## Decision

Stim is integrated via CMake **FetchContent at exact tag `v1.15.0`** (pinned —
do not track main, do not use `>= v1.x`). The library is consumed via the
`libstim` target. SIMD width is pinned project-wide to `-DSIMD_WIDTH=64` for
golden-test reproducibility (AVX vs SSE seeds give different shots).

```cmake
include(FetchContent)
set(SIMD_WIDTH 64 CACHE STRING "" FORCE)  # for golden reproducibility
FetchContent_Declare(stim
    GIT_REPOSITORY https://github.com/quantumlib/Stim.git
    GIT_TAG        v1.15.0
    GIT_SHALLOW    TRUE)
FetchContent_MakeAvailable(stim)
target_link_libraries(qfault_oracle PRIVATE libstim)
```

**Equivalence checking strategy:**

1. **Primary: `stim::Circuit::has_flow(stim::Flow)`** (added in Stim v1.13,
   stable through v1.15). A `Flow` is `(input PauliString → output PauliString,
   measurement set, observable set)`. Define logical-level flows the Stage 3
   output should satisfy (e.g., for BV-10 acting as a Z-oracle: 10 flows of
   form `Z_i → Z_i` plus X-flows mediated by the secret string), then call
   `has_flow` on both circuits A and B. With `unsigned_check=false`, gives a
   256-trial randomized check with 2⁻²⁵⁶ false-positive rate.

2. **Backstop: noiseless detector-distribution comparison.** Strip noise via
   `circuit.without_noise()`, assert
   `circuit.compute_stats().count_determined_measurements() == num_measurements()`,
   then run 1024 sweep-bit-driven shots — both circuits must produce zero
   detector flips and matching observable parities.

**Stim-out / Stim-in pinning:**
- Stim must be a **private detail** of `verify/` and `tests/` — never in the
  public include path of QFault. (Stim's instability would otherwise leak.)
- Include via `#include "stim.h"` (umbrella, quoted; Stim sets `src/` as the
  public include root).
- Use the alias `stim::MAX_BITWORD_WIDTH` for the SIMD template parameter
  rather than a hardcoded width.

## Alternatives Considered

| Alternative | Why Rejected |
|---|---|
| Stim via subprocess (call CLI) | High latency per call; not a CI-time-budget option |
| Stim via Python (pybind11 bridge) | Adds Python to verify/ build; defeats the C++-native pitch |
| Stim main branch instead of pinned tag | C++ ABI is openly unstable; goldens would drift |
| Auto-detect SIMD width | AVX vs SSE seeds give different sample bits → goldens platform-dependent |

## Consequences

**Positive:** Stim is a production-grade Pauli-frame simulator maintained by
Google Quantum. The CI gate inherits its correctness reputation. `has_flow` is
the cleanest API for stabilizer-circuit equivalence at QFault's granularity.

**Negative / Trade-offs:** SIMD-width=64 is slower than SIMD=256 by ~4× on
modern x86; for goldens this is acceptable (small reference circuits), but
production benchmarking should use a separate SIMD-256 build target.

**Risks:** Pinned tag v1.15.0 will go stale — budget for one tag-bump cycle
every 6–9 months. Each bump is a new ADR with measured equivalence-check delta
on the Stage 3 reference circuits.

## Implementation Notes for AI Sessions

When loading the memory bank:

- **Stim version is `v1.15.0`** in `CMakeLists.txt`. Bumping requires a new ADR.
- **Library target is `libstim`** (not `stim` — that's the CLI binary).
- **Include via `#include "stim.h"`** (umbrella, quoted).
- **SIMD width pinned to 64 for goldens** via `-DSIMD_WIDTH=64`.
- **Stim is private to `verify/` and `tests/`** — never in public headers.

**BV-10 at d=5 reference (Stage 3 gate):**
- 11 logical qubits (10 inputs + 1 ancilla)
- ~539–800 physical qubits (depends on routing)
- ~1000–1250 detectors
- 10 observables (one per input qubit Z)

**Stim pitfalls — never do these (CI failures observed in research):**
- Empty `REPEAT(0) { ... }` — illegal, parser rejects
- First-round detector referencing prior-round measurements
- Mixing SIMD widths across translation units → ODR violation, segfault at runtime
- `OBSERVABLE_INCLUDE` referencing detector indices instead of measurement records
- `circuit.detector_error_model()` on noiseless circuits — returns empty DEM,
  fails CI assertions; add at least one tiny noise channel for DEM extraction
- Forgetting `circuit.without_noise()` before equivalence comparison — makes
  equivalence appear stochastically broken

**Generate noiseless reference patches:**
```cpp
auto params = stim::CircuitGenParameters{
    /*rounds=*/d,
    /*distance=*/d,
    /*task=*/"surface_code:rotated_memory_z",
};
params.before_round_data_depolarization = 0;
params.before_measure_flip_probability  = 0;
params.after_reset_flip_probability     = 0;
params.after_clifford_depolarization    = 0;
auto reference = stim::generate_surface_code_circuit(params);
```

## References

- Stim, https://github.com/quantumlib/Stim, v1.15.0
- Stim developer documentation: `doc/developer_documentation.md`
- Stim file format documentation: `doc/file_format_stim_circuit.md`
- ADR-0009 (validation strategy), ADR-0017 (reproducibility infrastructure)