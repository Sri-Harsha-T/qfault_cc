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
- [x] 17 GitHub issues created (#29–#45) across 4 epics (A/B/C/D)
- [x] `cmake/dependency_versions.cmake` wired into `CMakeLists.txt` (issue #29)
- [x] QFAULT_ENABLE_STIM / QFAULT_ENABLE_QCEC options added to CMakeLists.txt
- [x] `cmake/stim_config.cmake` and `cmake/qcec_config.cmake` already written
- [ ] Stim FetchContent integration (#30) and oracle test harness (#31)
- [ ] MQT QCEC FetchContent + bridge (#34, #35)
- [ ] Benchmark corpus submodules (#38, #39) and harnesses (#41, #42)
- [ ] Reproducibility: Dockerfile (#43), flake.nix (#44), bench/Makefile (#45)

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
