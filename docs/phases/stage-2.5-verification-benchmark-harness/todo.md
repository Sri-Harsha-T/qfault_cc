# Stage 2.5 TODO — Verify + Bench

> Daily inner-loop tracker. Append-only with checkboxes.
> Status: planning complete; implementation pending.

## Pre-work: confirm Stage 2 closed

- [ ] Stage 2 exit report written: `docs/phases/stage-2-synthesis/exit-report.md`
- [ ] All 10 Stage 2 issues (#19–#28) closed on GitHub.
- [ ] CI green on `main`: 211/211 tests on gcc-13 + clang-18 + ASAN.
- [ ] ADRs 0002, 0003, 0004 all Accepted (with limitation note on 0004).
- [ ] ADRs 0009, 0017 Accepted (read before kicking off).

## Epic 2.5-D: Single source of truth (do first)

- [ ] 2.5-D-3: `cmake/dependency_versions.cmake` with all version pins.

## Epic 2.5-A: Stim integration

- [ ] 2.5-A-1: `cmake/stim_config.cmake`, FetchContent v1.15.0, link `libstim`.
- [ ] 2.5-A-2: `include/qfault/oracle/StimOracle.hpp`,
              `tests/integration/test_stim_oracle.cpp`.
- [ ] 2.5-A-3: `tests/integration/test_stim_detector_dist.cpp`.
- [ ] 2.5-A-4: SIMD-width `static_assert`s; `MAX_BITWORD_WIDTH` discipline.

## Epic 2.5-B: MQT QCEC bridge

- [ ] 2.5-B-1: `cmake/qcec_config.cmake`, FetchContent v3.5.0,
              `BUILD_MQT_QCEC_BINDINGS=OFF`.
- [ ] 2.5-B-2: `include/qfault/oracle/QCECBridge.hpp`,
              `tests/integration/test_qcec_bridge.cpp`.
- [ ] 2.5-B-3: Qubit-threshold dispatch; exit-code mapping.
- [ ] 2.5-B-4: 5 golden circuits under `bench/golden/qcec/`.

## Epic 2.5-C: Benchmark corpus

- [ ] 2.5-C-1: QASMBench submodule (shallow).
- [ ] 2.5-C-2: Feynman submodule + `dotqc_to_qasm.sh` adapter.
- [ ] 2.5-C-3: `bench/scripts/gen_mqtbench.py`.
- [ ] 2.5-C-4: Tier 1 single-rotation harness.
- [ ] 2.5-C-5: Tier 2 algorithm-level harness.

## Epic 2.5-D: Reproducibility infrastructure (continued)

- [ ] 2.5-D-1: `Dockerfile` multi-stage, pinned by digest.
- [ ] 2.5-D-2: `flake.nix` + `flake.lock`.
- [ ] 2.5-D-4: `bench/Makefile` with `figures-tier1`, `figures-tier2`,
              `figures` targets.

## Stage Gate

- [ ] ctest green: clang-18 debug + release with Stim/QCEC integration.
- [ ] ctest green: gcc-13 debug + release.
- [ ] ASAN + UBSAN clean on Stim oracle and QCEC bridge tests.
- [ ] clang-tidy clean on all new files.
- [ ] `docker build .` succeeds on clean Linux.
- [ ] `nix build` succeeds on clean machine with Nix.
- [ ] `make figures` reproduces `bench/golden/stage2_baseline.csv` within 5%.
- [ ] activeContext.md "Next Action" set to Stage 3.
- [ ] progress.md Stage 2.5 row updated.
- [ ] CHANGELOG.md updated.
- [ ] GitHub Milestone "Stage 2.5" closed.
- [ ] exit-report.md written.
