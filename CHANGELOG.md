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

### Stage 2: Synthesis Pass (T-Gate)
- [ ] `SynthesisProvider` interface defined
- [ ] GridSynth wrapped as primary provider
- [ ] Solovay-Kitaev as baseline/benchmark provider
- [ ] T-count validated against published GridSynth benchmarks
- [ ] Stage 2 gate: abstraction overhead ≤5% vs direct GridSynth call on 1000-gate circuit

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
