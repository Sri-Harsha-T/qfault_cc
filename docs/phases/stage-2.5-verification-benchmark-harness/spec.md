# Stage 2.5 Spec: Verification and Benchmark Harness

**Stage:** 2.5 of 7 (inserted between Stage 2 and Stage 3)
**Target duration:** 4–6 weeks
**GitHub Milestone:** "Stage 2.5: Verify + Bench"
**Depends on:** Stage 2 complete

---

## Goal

Build the plumbing that Stage 3's "Stim says correct" gate depends on, and
the benchmark harness that Stages 5c, 6, and 7 will all consume. Without
this stage, Stage 3 has nowhere to write Stim oracles, no MQT QCEC bridge,
and no benchmark corpus.

**Stage Gate (hard stop before Stage 3):**

> Running `make figures` on a clean `ubuntu:24.04` Docker container
> reproduces the published Stage 2 numbers within 5%, with all external
> binaries pinned by version. `tests/integration/test_stim_oracle.cpp`
> and `tests/integration/test_qcec_bridge.cpp` both pass against
> committed golden circuits.

---

## Why this stage exists (rationale)

The original 5-stage plan jumped from Stage 2 (synthesis) to Stage 3
(lattice surgery routing) with the gate "Stim oracle confirms correct
logical output". That gate has no plumbing in Stage 2's code — Stim is
not yet linked, MQT QCEC is not yet wired, no golden circuits exist. The
audit identified this as a gap; Stage 2.5 fills it.

Per ADR-0009 (verification strategy), the validation layer comprises Stim
+ QCEC + golden-circuit regression. This stage builds all three.

Per ADR-0017 (reproducibility infrastructure), the artifact bundle
requires Dockerfile + flake.nix + papers/ scaffolding. This stage starts
that infrastructure.

---

## Epics and Stories

### Epic 2.5-A: Stim integration

| Story | Acceptance Criteria | Est. |
|-------|---------------------|------|
| 2.5-A-1: Stim FetchContent integration | `cmake/stim_config.cmake` declares `FetchContent_Declare(stim)` pinned to `v1.15.0` with `SIMD_WIDTH=64`. `target_link_libraries(qfault_oracle PRIVATE libstim)`. `make` succeeds on clean Ubuntu 24.04. | 2d |
| 2.5-A-2: Stim oracle test harness | `tests/integration/test_stim_oracle.cpp` defines a `StimOracle` helper that takes a `QFaultIRModule` (Clifford segment), emits the equivalent Stim circuit, and uses `stim::Circuit::has_flow` for equivalence checks. Test passes for a 3-qubit Clifford-only circuit. | 3d |
| 2.5-A-3: Detector-distribution backstop | Add `tests/integration/test_stim_detector_dist.cpp` using `circuit.without_noise()` + 1024 sweep-bit shots. Both circuits must produce zero detector flips and matching observable parities. | 2d |
| 2.5-A-4: SIMD-width discipline | All Stim-touching files use `stim::MAX_BITWORD_WIDTH`. Add a CMake check + `static_assert` in the oracle helper that the width matches across translation units. | 1d |

### Epic 2.5-B: MQT QCEC bridge

| Story | Acceptance Criteria | Est. |
|-------|---------------------|------|
| 2.5-B-1: QCEC FetchContent integration | `cmake/qcec_config.cmake` declares `FetchContent_Declare(qcec)` pinned to `v3.5.0` with `BUILD_MQT_QCEC_BINDINGS=OFF`. `make` succeeds. | 2d |
| 2.5-B-2: QCEC bridge helper | `tests/integration/test_qcec_bridge.cpp` defines a `QCECBridge` helper using `EquivalenceCheckingManager`. Construction checker OFF; Alternating + Simulation + ZX ON. | 2d |
| 2.5-B-3: Qubit-threshold dispatch | Bridge logic: ≤ 8 qubits demand `Equivalent` or `EquivalentUpToGlobalPhase`; > 8 qubits accept `ProbablyEquivalent`. Map exit codes 0/2/3/4 = pass/not-eq/probably-not-eq/no-info. | 1d |
| 2.5-B-4: QCEC golden circuits | Commit 5 reference circuits under `bench/golden/qcec/`: BV-{4,6,8}, QFT-4, simple adder. Each has `before.qasm` (input) and `expected_verdict.txt` (one of `equivalent` / `equivalent_up_to_global_phase` / etc.). | 2d |

### Epic 2.5-C: Benchmark corpus

| Story | Acceptance Criteria | Est. |
|-------|---------------------|------|
| 2.5-C-1: QASMBench submodule | `git submodule add` PNNL/QASMBench under `bench/circuits/qasmbench` (shallow). `git submodule update --init --recursive --depth 1` works. CI does not use `--remote`. | 1d |
| 2.5-C-2: Feynman corpus submodule | `git submodule add` Feynman benchmarks. Add a `bench/scripts/dotqc_to_qasm.sh` adapter for Feynman's `.qc` format. | 1d |
| 2.5-C-3: MQT Bench generator wrapper | `bench/scripts/gen_mqtbench.py` invokes the MQT Bench Python generator, writing OpenQASM 3.0 to `bench/_generated/` (gitignored). README documents the regeneration workflow. | 1d |
| 2.5-C-4: Tier 1 single-rotation harness | `bench/tier1/run.sh` executes 10⁴ random angles + Ross-Selinger Table 1 references through GridSynthProvider, BFSTableProvider (renamed SKProvider), rsgridsynth, newsynth, pygridsynth. Outputs CSV to `bench/results/tier1.csv`. | 4d |
| 2.5-C-5: Tier 2 algorithm-level harness | `bench/tier2/run.sh` runs the QASMBench small/ directory through QFault end-to-end. Records T-count, gate count, depth, compile time. CSV output. | 3d |

### Epic 2.5-D: Reproducibility infrastructure

| Story | Acceptance Criteria | Est. |
|-------|---------------------|------|
| 2.5-D-1: Dockerfile multi-stage | `Dockerfile` at repo root, multi-stage: `builder` (Ubuntu 24.04 + gcc-13 + clang-18 + cmake + Haskell + gridsynth build) and `runtime` (slim image with built artifacts). Pinned by digest. `docker build` succeeds. | 4d |
| 2.5-D-2: flake.nix | `flake.nix` and committed `flake.lock`. `nix build` produces a working `qfault_tests` binary. nixpkgs commit pinned. | 3d |
| 2.5-D-3: cmake/dependency_versions.cmake | Single source of truth for all version pins (Stim, QCEC, GoogleTest, Benchmark, pybind11, gridsynth). Read by `CMakeLists.txt` and exported as Docker build args. | 1d |
| 2.5-D-4: make figures targets | `bench/Makefile` with `figures-tier1`, `figures-tier2`, `figures` targets. Outputs PDFs to `bench/plots/`. Uses matplotlib via `bench/scripts/plot.py`. | 2d |

---

## Stage 2.5 Definition of Done

- [ ] All story ACs pass (`ctest -j` green on clang-18 AND gcc-13 in
      debug and release).
- [ ] ASAN + UBSAN clean on all Stim/QCEC integration tests.
- [ ] `clang-tidy` clean on all new `src/` and `tests/integration/` files.
- [ ] `docker build .` succeeds on a clean Linux machine.
- [ ] `nix build` succeeds on a clean machine with Nix installed.
- [ ] `make figures` reproduces Stage 2 numbers within 5% on the Docker
      container (acceptance: numbers in `bench/golden/stage2_baseline.csv`
      vs `bench/results/tier1.csv` within 5% relative diff).
- [ ] All Stim/QCEC version pins in `cmake/dependency_versions.cmake`.
- [ ] `memory-bank/activeContext.md` updated with Stage 3 "Next Action".
- [ ] `memory-bank/progress.md` Stage 2.5 row marked complete with date.
- [ ] `CHANGELOG.md` updated.
- [ ] GitHub Milestone "Stage 2.5" closed; all issues Done.

---

## Source Layout (Stage 2.5 additions)

```
cmake/
  stim_config.cmake                    # FetchContent pin for Stim v1.15.0
  qcec_config.cmake                    # FetchContent pin for MQT QCEC v3.5.0
  dependency_versions.cmake            # single source of truth
  qir_version.cmake                    # QIR Alliance v0.1 base profile (used in Stage 5a)

include/qfault/oracle/
  StimOracle.hpp                       # Stim-based equivalence helper
  QCECBridge.hpp                       # MQT QCEC bridge helper

src/qfault/oracle/
  StimOracle.cpp
  QCECBridge.cpp

tests/integration/
  test_stim_oracle.cpp
  test_stim_detector_dist.cpp
  test_qcec_bridge.cpp

bench/
  circuits/                            # git submodules (shallow)
    qasmbench/
    feynman/
  _generated/                          # gitignored, regenerated MQT Bench
  golden/                              # committed reference outputs
    stage2_baseline.csv
    qcec/
      bv_n4.qasm
      ...
  tier1/run.sh
  tier2/run.sh
  scripts/
    gen_mqtbench.py
    dotqc_to_qasm.sh
    plot.py
  Makefile
  results/                             # gitignored CSV outputs
  plots/                               # gitignored PDF outputs

Dockerfile
flake.nix
flake.lock
```

---

## Risks and Mitigations

| Risk | Mitigation |
|------|-----------|
| Stim's unstable C++ ABI breaks between v1.15.0 and v1.16 | Pin to v1.15.0 in `cmake/stim_config.cmake`; budget tag-bump every 6–9 months |
| QCEC times out on Clifford+T blocks > 8 qubits | Bridge logic falls back to `ProbablyEquivalent` (per ADR-0009) |
| Docker base image churn | Pin by digest; pinned digest stored in `cmake/dependency_versions.cmake` |
| nixpkgs commit pinning lags security patches | Acceptable for research artifact; document in `flake.nix` |
| MQT Bench generator API drift | The generator is invoked from a script, isolated from QFault internals |
