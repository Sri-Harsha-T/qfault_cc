# QFault Changelog

All notable changes are appended here chronologically.
This file is read at the start of every Claude Code session.
The "Failed Approaches" section is **mandatory reading** — do not retry listed ideas.

---

## Failed Approaches — DO NOT RETRY

> These have been explored and rejected. Each entry explains why, so future
> sessions understand the reasoning and don't re-propose the same dead ends.

| Date | Approach | Why It Failed | Alternative Used |
|------|----------|---------------|------------------|
| 2026-04-25 | Omitting `std::optional` fields in designated-init aggregate construction | gcc-13 `-Wmissing-field-initializers` fires even when the default is correct (e.g. `LogicalGate{.kind=T, .operands={q}}` omits `.angle`). clang-18 is silent. | Add `= std::nullopt` as a default member initializer in the struct; then the field need not be listed in designated inits |
| 2026-04-25 | `volatile int sum` spin loop in `test_PassContext::TimerMeasuresElapsedTime` | `sum(0..99999) = 4,999,950,000` exceeds `INT_MAX`; UBSAN reports signed integer overflow at `i=65536` (`2147450880 + 65536`). clang-18 + UBSAN with `halt_on_error=1` aborts the test. | Changed to `volatile long long sum` — the value fits without overflow |
| 2026-04-25 | Generic `debug` CMake preset on a machine with system gcc 9.4 | gcc 9.4 does not support C++20 `= default operator==`; compilation fails with "cannot be defaulted". The `debug` preset has no explicit compiler. | Always use named presets: `gcc13-debug` or `clang18-debug`. System compiler is only used for non-C++20 projects. |
| 2026-04-25 | `scripts/quick-test.sh` using `build/debug` (system gcc 9.4) | Same as above — script hardcoded the `debug` preset. `./scripts/quick-test.sh` failed with `= default operator==` errors. | Fixed script to use `gcc13-debug` by default; overridable via `QFAULT_PRESET` env var. |
| 2026-04-25 | Spec integration test assertion "no T/Tdg remain after TGateSynthesisPass<SKProvider>" | Physically wrong: SKProvider returns `{T}` for R_z(π/4) because T is the exact answer. After replacement, T gates are still present. | Test uses a `CliffordOnlyProvider` mock (returns `{H,S,H}`) to verify the pass mechanism; separate test checks SKProvider doesn't crash. Real "no T remain" only holds for GridSynth with a Clifford-only output — not for π/4 which IS T. |
| 2026-04-28 | `gridsynth -- angle -e eps` argument order in GridSynthProvider | gridsynth CLI requires options BEFORE the angle positional argument; `--` stops option parsing so `-e` after `--` is not recognized, giving "Too many non-option arguments" error and returning empty sequence. | Fixed to `gridsynth -e eps -p angle`; also added `-p` (global phase) since phase is irrelevant in QEC, and `std::setprecision(max_digits10)` to pass full double precision. |
| 2026-04-28 | `test_TCountValidation` reference T-counts (3, 7, 9, 13, 7) for eps=1e-10 | Values were wrong — these T-counts are for coarse eps (~1e-1), not 1e-10. At eps=1e-10, newsynth gives T-count≈100 for generic angles (lower_bound == T-count; provably optimal). The paper Table 1 was misread. | Updated to actual values: pi/4→1, pi/8→98, pi/16→100, pi/32→100, 3pi/8→98. Also found `std::ostringstream` default 6-sig-fig precision was giving gridsynth a wrong angle (pi/32: 0.0981748 vs 0.09817477042468103), producing a non-optimal factorization. |
| 2026-05-17 | `std::expected` in C++20 mode with gcc-13 / Stim build | `std::expected` is C++23; the project was set to `CMAKE_CXX_STANDARD 20`. The oracle and its tests use `std::expected` throughout; gcc-13 refuses to compile it in C++20 mode. | Bumped project to C++23 globally. Both gcc-13 and clang-18 fully support C++23 including `std::expected`. |
| 2026-05-17 | Stim v1.15.0 `stim_perf` linker error under C++23 | `stim_perf` has an undefined reference to `stim::biased_randomize_bits` in its perf harness when built as part of ALL. This is an internal Stim issue unrelated to our code. | Exclude all non-library Stim targets (`stim`, `stim_perf`, `stim_python_bindings`) from ALL via `EXCLUDE_FROM_ALL TRUE`; only `libstim` is built. |
| 2026-05-17 | `-Wdeprecated-copy` / `-Wunused-parameter` from Stim v1.15.0 templates in our TUs | Stim's internal template code (tableau_transposed_raii, etc.) has deprecated-copy patterns that fire as errors when our TUs instantiate `circuit_to_tableau<W>` under -Werror. | Added `#pragma GCC diagnostic push/pop` around Stim includes in StimOracle.hpp/cpp and detector test; also added `-Wno-deprecated-copy` to `qfault_link_stim()` targets. |
| 2026-05-17 | `SIMD_WIDTH=64` not propagating to our TUs → `stim::MAX_BITWORD_WIDTH = 128` | Setting `SIMD_WIDTH` as a CMake cache variable before `FetchContent_MakeAvailable(stim)` sets it for Stim's own builds, but our TUs include `stim.h` directly and get MAX_BITWORD_WIDTH from the CPU's native width (128 on AVX2). `-DSIMD_WIDTH=64` was not added to our compile definitions. | Hardcoded `constexpr std::size_t kStimW = 64` in oracle and test files; use `kStimW` as template parameter directly. Removed `stim::MAX_BITWORD_WIDTH` as template param (not `static_assert`). |
| 2026-05-17 | Empty Stim circuit (no instructions) has `num_qubits = 0` → tableau comparison fails | For `circuits_clifford_equivalent(H;H, identity)`, the identity module has no instructions. The Stim circuit text is empty → 0-qubit circuit → 0×0 tableau ≠ 1×1 tableau for H;H even though both are identity. | In `circuits_clifford_equivalent`, if the Stim text is empty but qubits exist, prepend `I 0 1 ... n-1` to anchor the qubit count without changing the stabiliser tableau. Applied to BOTH circuits before tableau comparison. |
| 2026-05-17 | `QFAULT_HAS_STIM` not visible in `qfault_tests` despite Stim being enabled | The compile definition `QFAULT_HAS_STIM=1` was PRIVATE to `qfault_oracle`. The test binary linked against `qfault_oracle` but did NOT see the definition; `#ifdef QFAULT_HAS_STIM` in test files compiled the GTEST_SKIP() branch. | In CMakeLists.txt, always call `qfault_link_stim(qfault_tests)` (and same for QCEC) when enabled, regardless of whether `qfault_oracle` exists. This propagates definitions to the test binary. |
| 2026-05-17 | `qasm3::Importer::imports` undefined reference in qfault_tests link | `mqt-core-qasm` (the qasm3 parser library) is not exported as a transitive dependency of `MQT::QCEC`. Linking against `MQT::QCEC` does not pull in the importer symbol. | Added `if(TARGET mqt-core-qasm) target_link_libraries(${target} PRIVATE mqt-core-qasm)` to `qfault_link_qcec()` in qcec_config.cmake. |

**Template for new entries:**
```
| YYYY-MM-DD | <what was tried> | <why it failed — be specific> | <what replaced it> |
```

---

## Stage Progress Log

### Stage 1: IR + Pass Manager Core ✅ COMPLETE (2026-04-25)
- [x] Kickoff complete
- [x] `QFaultIR` data structures defined (LogicalGate, PatchOp, QFaultIRModule)
- [x] `PassManager` skeleton implemented (add<T>(), run(), printStats())
- [x] No-op pass chain with full test harness (93 tests, gcc-13 + clang-18)
- [x] Stage 1 gate: IR can represent both logical Clifford+T AND surface code patch ops
- [x] QASM 3.0 Lexer + Parser (Clifford+T subset) with round-trip test
- [x] GitHub Actions CI (4-way matrix + ASAN job)
- [x] UBSAN overflow fix in test_PassContext

### Stage 2: Synthesis Pass (T-Gate) — Code Complete (2026-04-25)
- [x] `SynthesisProvider` C++20 Concept defined (#19)
- [x] `GridSynthProvider` subprocess wrapper (#20)
- [x] `SKProvider` Solovay-Kitaev pure C++, depth-7 BFS (#21)
- [x] `TGateSynthesisPass<Provider>` template pass + integration test (#23 #24)
- [x] CMake GridSynth optional dependency + GTEST_SKIP() guards (#26 #27)
- [x] T-count validation test (skipped without binary) (#25)
- [x] `scripts/bench-synthesis.sh` overhead benchmark (#28)
- [x] Stage 2 gate (2026-04-28): T-count within 1% ✅ (fixed arg order + -p + precision); bench baseline 3633ms / 300 T-gates ✅

### Stage 2.5: Verify + Bench (started 2026-04-28)
- [x] GitHub milestone #11 "Stage 2.5: Verify + Bench" created
- [x] 17 GitHub issues created (#29–#45) across 4 epics (A/B/C/D) + issue #46 (CI)
- [x] `cmake/dependency_versions.cmake` wired into `CMakeLists.txt` (issue #29, closed)
- [x] QFAULT_ENABLE_STIM / QFAULT_ENABLE_QCEC options added to CMakeLists.txt
- [x] `cmake/stim_config.cmake` and `cmake/qcec_config.cmake` written and working
- [x] `gcc13-stim` / `clang18-stim` CMake presets (issue #46 partial; CI job still needed)
- [x] Stim v1.15.0 FetchContent integration — 139/139 tests pass (issue #30, closed 2026-05-17)
- [x] StimOracle helper: `ir_to_stim_text()` + `circuits_clifford_equivalent()` (issue #31, closed)
- [x] Detector-distribution backstop: 4 tests on reference samples + 1024-shot sweep (issue #32, closed)
- [x] SIMD-width discipline: `kStimW=64`, static_assert, pragma guards (issue #33, closed)
- [x] MQT QCEC v3.5.0 FetchContent integration (issue #34, closed 2026-05-17)
- [x] QCECBridge: `check_equivalence()`, `is_passing()`, `EquivalenceResult` enum (issue #35, closed)
- [x] Qubit-threshold dispatch: `kQcecStrictThreshold=8` (issue #36, closed 2026-05-17)
- [x] Project bumped to C++23 globally (gcc-13 + clang-18 fully support; required for std::expected)
- [ ] QCEC golden circuits: bench/golden/qcec/ (issue #37)
- [ ] Benchmark corpus submodules (#38, #39) and harnesses (#41, #42)
- [ ] bench/scripts utilities (#40, #45)
- [ ] Reproducibility: Dockerfile (#43), flake.nix (#44)
- [ ] CI Stim/QCEC integration job (issue #46 remainder)

### Stage 3: Lattice Surgery Mapper
- [ ] Logical CNOT → patch merge/split sequences
- [ ] Greedy spatial routing heuristic
- [ ] 10-qubit Bernstein-Vazirani validated against Stim at d=5
- [ ] Stage 3 gate: Stim oracle confirms correct logical output

### Stage 4: MSD Scheduling + Resource Estimator
- [ ] MSD factory modelled as spatial reservation
- [ ] T-gate scheduler with routing delays
- [ ] `ResourceEstimator` API (physical qubits, time-steps, code distance)
- [ ] Stage 4 gate: beats naïve "one factory per T-gate" on ≥50 T-gate circuits

### Stage 5: Output Backends + Python Bindings + arXiv
- [ ] QASM 3.0 output backend
- [ ] QIR output backend (pinned spec version)
- [ ] pybind11 Python bindings + PyPI package
- [ ] arXiv preprint with end-to-end benchmarks
- [ ] v0.1.0 GitHub release

---

## Change Log

### [Unreleased] — Stage 1 in progress

#### Added
- Initial project skeleton and documentation system
- CLAUDE.md, memory-bank/, docs/adr/, docs/phases/ scaffolding
- ccpm integration in .claude/skills/ccpm

---
