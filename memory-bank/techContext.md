# Tech Context — QFault

## Language & Standard
- **C++20** — Concepts, ranges, `std::span`, designated initialisers, `consteval`
- Both **clang-17+** and **gcc-13+** must build and pass tests cleanly
- `-Wall -Wextra -Wpedantic` with zero warnings; `-Werror` in CI

## Build System
- **CMake ≥ 3.16** with modern target-based dependency graph
- Presets in `CMakePresets.json`: `debug`, `release`, `asan`, `coverage`
- Build tree: `build/` (gitignored); never in-source builds

```cmake
# Key CMake flags to know
-DQFAULT_ENABLE_ASAN=ON        # Address sanitizer build
-DQFAULT_ENABLE_UBSAN=ON       # UB sanitizer build
-DQFAULT_RUN_BENCHMARKS=ON     # Enables Google Benchmark targets
-DQFAULT_ENABLE_PYTHON=ON      # Builds pybind11 bindings (Stage 5)
-DQFAULT_GRIDSYNTH_PATH=<path> # Path to GridSynth binary/lib
```

## Core Dependencies

| Dependency | Version | Purpose | How acquired |
|-----------|---------|---------|-------------|
| GoogleTest | 1.14+ | Unit + integration tests | CMake FetchContent |
| Google Benchmark | 1.8+ | Micro-benchmarks | CMake FetchContent |
| GridSynth | latest | T-optimal single-qubit synthesis (wrapped) | System install or FetchContent |
| Stim | 1.14+ | Simulation oracle for validation | System install (validation only) |
| pybind11 | 2.10+ | Python bindings (Stage 5 only) | CMake FetchContent |

**No LLVM dependency for v0.1** — MLIR/QIR integration considered for v0.2.
QIR output in v0.1 is generated directly without the LLVM toolchain.

## Static Analysis & Sanitizers
```bash
# Must pass before any PR merge
clang-tidy --config-file=.clang-tidy src/
cppcheck --enable=all --suppress=missingInclude src/
```

`.clang-tidy` config: enable `modernize-*`, `performance-*`, `readability-*`,
`cppcoreguidelines-*`. Disable `fuchsia-*` (overly restrictive).

## Python Bindings (Stage 5)
- **pybind11** (not nanobind for now — broader community familiarity)
- Python ≥ 3.10
- Package name on PyPI: `qfault`
- Built via `scikit-build-core` in `pyproject.toml`

## External Oracle: Stim
Stim is used ONLY for validation — never as a runtime dependency.
```bash
# Install: pip install stim
# Use in tests: tests/integration/run_stim_oracle.py
```
When QFault output diverges from Stim oracle, Stim is correct.

## Quantum Interchange Formats
- **Input:** OpenQASM 3.0 (primary), Cirq JSON (secondary for testing)
- **Output Stage 5a:** OpenQASM 3.0 (`qasm3` backend)
- **Output Stage 5b:** QIR v1.0 draft (pinned — see ADR-0005 when written)

## Repository Tooling
- **GitHub Issues + Projects v2** — 5 Milestones (one per stage)
- **ccpm** in `.claude/skills/ccpm` — PRD→epic→issue decomposition
- **GitHub Actions CI:** build matrix (clang/gcc × debug/release/asan), ctest
- **Pre-commit hooks:** clang-format, trailing whitespace, CHANGELOG check

## Directory Layout (source)
```
src/
  qfault/
    ir/           # QFaultIR data structures (Stage 1)
    passes/       # PassManager + all compiler passes
      synthesis/  # SynthesisProvider interface + GridSynth/SK providers
      lattice/    # LatticeSurgeryPass (Stage 3)
      msd/        # MSDSchedulerPass (Stage 4)
      estimate/   # ResourceEstimatorPass (Stage 4)
    frontend/     # QASM 3.0 parser (Stage 1, basic)
    backend/      # QASM 3.0 + QIR emitters (Stage 5)
    util/         # Shared utilities, logging, error types
include/
  qfault/         # Public headers only (pimpl where appropriate)
tests/
  unit/
  integration/
  benchmarks/
  reference/
bindings/
  python/         # pybind11 bindings (Stage 5)
scripts/
  quick-test.sh
  full-build.sh
  asan-ubsan.sh
  compare-stim.sh
  benchmark.sh
```
