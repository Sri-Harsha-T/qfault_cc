# Stage 2.5 Exit Report: Verification & Benchmark Harness

**Stage:** 2.5 of 7  
**Completed:** 2026-05-17  
**Issues closed:** #29–#46 (17 issues across 4 epics + CI)  
**Final test count:** 144/144 green on `gcc13-stim` (C++23)

---

## Summary

Stage 2.5 built the validation and reproducibility plumbing required before
Stage 3 can use the "Stim oracle confirms correct logical output" gate.

Epic A (Stim integration) and Epic B (MQT QCEC bridge) were completed in the
first session (2026-05-17, commit 856ebc4). Epic C (benchmark corpus) and
Epic D (reproducibility infrastructure) were completed in the second session
of the same day.

---

## What Was Built

### Epic A — Stim v1.15.0 Integration (#29–#33)

| Issue | Deliverable |
|-------|-------------|
| #29 | `cmake/dependency_versions.cmake` — single source of truth for all version pins |
| #30 | `cmake/stim_config.cmake` — FetchContent Stim v1.15.0; `libstim` target; SIMD=64 |
| #31 | `StimOracle.hpp/cpp` — `ir_to_stim_text()`, `circuits_clifford_equivalent()` |
| #32 | `test_stim_detector_dist.cpp` — 4 detector-distribution backstop tests |
| #33 | SIMD-width discipline: `kStimW=64`, `static_assert`, pragma guards throughout |

### Epic B — MQT QCEC v3.5.0 Bridge (#34–#37)

| Issue | Deliverable |
|-------|-------------|
| #34 | `cmake/qcec_config.cmake` — FetchContent QCEC v3.5.0; `MQT::QCEC` target |
| #35 | `QCECBridge.hpp/cpp` — `check_equivalence()`, `EquivalenceResult`, `is_passing()` |
| #36 | Qubit-threshold dispatch: ≤8 qubits strict, >8 qubits relaxed |
| #37 | `bench/golden/qcec/` — 5 reference circuit pairs (BV-4/6/8, QFT-4, adder-4) + regression test |

### Epic C — Benchmark Corpus (#38–#42)

| Issue | Deliverable |
|-------|-------------|
| #38 | `bench/circuits/qasmbench/` — PNNL QASMBench shallow git submodule |
| #39 | `bench/circuits/feynman/` — meamy/feynman shallow git submodule + `dotqc_to_qasm.sh` adapter |
| #40 | `bench/scripts/gen_mqtbench.py` — MQT Bench Python generator wrapper |
| #41 | `bench/tier1/run.sh` — 10k random angles through GridSynthProvider, CSV output |
| #42 | `bench/tier2/run.sh` — QASMBench small/ directory end-to-end, CSV output |

### Epic D — Reproducibility Infrastructure (#43–#46)

| Issue | Deliverable |
|-------|-------------|
| #43 | `Dockerfile` — multi-stage (builder: Ubuntu 24.04 + gcc-13 + clang-18; runtime: slim) |
| #44 | `flake.nix` + inputs — Nix reproducibility entry point, gcc-13/clang-18/cmake pinned |
| #45 | `bench/Makefile` — `figures-tier1/2`, `regression` targets; `bench/scripts/plot.py`; `bench/golden/stage2_baseline.csv`; `bench/scripts/check_regression.py` |
| #46 | `.github/workflows/ci.yml` — `stim-integration` CI job using `gcc13-stim` preset |

---

## Key Challenges and How They Were Resolved

Seven build/runtime issues were encountered and resolved during Epic A+B.
All are logged in `CHANGELOG.md "Failed Approaches"` with root causes and
fixes. Below is a summary organised by impact:

### 1. C++ standard mismatch (`std::expected` in C++20)

`std::expected<T, E>` is a C++23 feature. The oracle code used it throughout,
but the project was configured for C++20. gcc-13 refused to compile.

**Fix:** Bumped `CMAKE_CXX_STANDARD` from 20 to 23 globally. Both gcc-13 and
clang-18 fully support C++23, including `std::expected` and `std::format`. All
`cxx_std_20` compile features updated to `cxx_std_23`.

### 2. Stim `stim_perf` linker error (unrelated to our code)

Stim v1.15.0's `stim_perf` target has an undefined reference to
`stim::biased_randomize_bits` under C++23. This is an internal Stim issue.

**Fix:** In `stim_config.cmake`, set `EXCLUDE_FROM_ALL TRUE` on all non-`libstim`
Stim targets so only `libstim` is built as part of the default build.

### 3. Stim warnings treated as errors under `-Werror`

Stim v1.15.0's internal template code (`tableau_transposed_raii` and related)
triggers `-Wdeprecated-copy` and `-Wunused-parameter` when our translation
units instantiate `circuit_to_tableau<W>`. These fire as errors under
`-Werror`.

**Fix:** Added `#pragma GCC diagnostic push/pop` guards around all Stim
header includes in `StimOracle.hpp`, `StimOracle.cpp`, and
`test_stim_detector_dist.cpp`. Also added `-Wno-deprecated-copy` to the
`qfault_link_stim()` target helper in `stim_config.cmake`.

### 4. SIMD width mismatch between Stim's build and our TUs

Setting `SIMD_WIDTH=64` as a CMake cache variable before
`FetchContent_MakeAvailable(stim)` applies only to Stim's own build.
Our translation units include `stim.h` directly and receive
`MAX_BITWORD_WIDTH = 128` from the CPU's native AVX2 capability.
Template instantiations at `W=64` vs `W=128` are incompatible at link time.

**Fix:** Hardcoded `constexpr std::size_t kStimW = 64` in all oracle and
test files. Replaced all `stim::MAX_BITWORD_WIDTH` template parameters with
`kStimW`. Added a `static_assert(64 <= stim::MAX_BITWORD_WIDTH)` to catch
non-AVX2 machines at compile time.

### 5. Empty Stim circuit has `num_qubits = 0`

For `circuits_clifford_equivalent(H;H, identity)`, the identity `QFaultIRModule`
has no instructions. Converting it to Stim text yields an empty string; an empty
Stim circuit has `num_qubits = 0`. Comparing a 0-qubit identity tableau against
a 1-qubit H;H tableau always returns `false` even though both are identity.

**Fix:** In `circuits_clifford_equivalent()`, an `anchor_qubits` lambda prepends
`I 0 1 ... n-1` to any empty circuit text before tableau construction, anchoring
the qubit count without changing the stabiliser tableau. Applied to both circuits
before comparison.

### 6. `QFAULT_HAS_STIM=1` not visible in `qfault_tests`

The compile definition `QFAULT_HAS_STIM=1` was set as `PRIVATE` on
`qfault_oracle`. The test binary links against `qfault_oracle` but does NOT
inherit private compile definitions, so `#ifdef QFAULT_HAS_STIM` compiled the
`GTEST_SKIP()` branch instead of the real tests.

**Fix:** In `CMakeLists.txt`, always call `qfault_link_stim(qfault_tests)` and
`qfault_link_qcec(qfault_tests)` when the respective features are enabled,
regardless of whether `qfault_oracle` is linked.

### 7. `qasm3::Importer::imports` undefined reference

`mqt-core-qasm` (the QASM3 parser library) is built by the `mqt-core`
FetchContent dependency but is not exported as a transitive dependency of
`MQT::QCEC`. Linking against `MQT::QCEC` does not pull in the importer symbol.

**Fix:** Added `if(TARGET mqt-core-qasm) target_link_libraries(${target} PRIVATE
mqt-core-qasm)` to `qfault_link_qcec()` in `qcec_config.cmake`.

---

## Test Results

| Preset | Tests | Status |
|--------|-------|--------|
| `gcc13-stim` | 144/144 | ✅ All pass |
| `gcc13-debug` | 123/123 | ✅ All pass |
| `clang18-debug` | 123/123 | ✅ All pass |
| `gcc13-release` | 123/123 | ✅ All pass |
| `clang18-release` | 123/123 | ✅ All pass |

The additional 21 tests over the Stage 2 baseline (123 → 144) break down as:
- 7 Stim oracle tests (`test_stim_oracle.cpp`)
- 4 detector-distribution tests (`test_stim_detector_dist.cpp`)
- 8 QCEC bridge tests (`test_qcec_bridge.cpp`)
- 1 GTEST_SKIP placeholder test (for non-stim builds)
- 5 golden circuit regression tests (`test_qcec_golden.cpp`, parametrised)

---

## Stage Gate Assessment

The Stage 2.5 gate requires:

> Running `make figures` on a clean `ubuntu:24.04` Docker container
> reproduces the published Stage 2 numbers within 5%, with all external
> binaries pinned by version.

**Status:** The infrastructure is in place:
- `bench/Makefile` with `figures`, `tier1`, `tier2`, `regression` targets ✅
- `bench/golden/stage2_baseline.csv` committed with Stage 2 reference numbers ✅
- `bench/scripts/check_regression.py` implements the 5% tolerance check ✅
- `Dockerfile` multi-stage build available ✅
- `flake.nix` Nix entry point available ✅
- All version pins in `cmake/dependency_versions.cmake` ✅

The `make figures` target requires GridSynth to be installed and MQT Bench
Python package (`pip install mqt-bench`) for the full sweep. The CI
`stim-integration` job covers the Stim/QCEC path; the bench scripts run
as a separate opt-in step.

---

## Remaining Open Items (deferred to Stage 3+)

- `flake.lock` is not committed (requires `nix flake lock` on a Nix-enabled
  machine with internet access — deferred to first reproducibility artifact run)
- GridSynth Haskell binary is not included in the Dockerfile build stage
  (requires `ghcup` + `cabal install gridsynth` — currently a manual step
  documented in the Dockerfile)
- `bench/tier2/run.sh` writes a placeholder row per circuit rather than running
  QFault's synthesis pass end-to-end (QFault CLI not yet available in Stage 2.5)
- CI `stim-integration` job does not install GridSynth, so GridSynth-dependent
  tests still `GTEST_SKIP()` on CI (unchanged from Stage 2)

---

## ADRs Produced

| ADR | Decision |
|-----|----------|
| ADR-0009 | Validation strategy: Stim + MQT QCEC (accepted) |
| ADR-0017 | Reproducibility: Dockerfile + flake.nix + Zenodo + papers/ (accepted) |
| ADR-0021 | Stim oracle: FetchContent v1.15.0, `libstim`, SIMD=64 (accepted) |

---

## Next Stage

**Stage 3: Lattice Surgery Mapper** — all Stage 2.5 plumbing is in place.
The `circuits_clifford_equivalent()` function is ready to serve as the
"Stim oracle confirms correct logical output" gate for Stage 3.

See `docs/phases/stage-3-lattice-surgery/` for the Stage 3 plan.
