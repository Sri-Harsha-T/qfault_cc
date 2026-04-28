# Tech Context — QFault

## Toolchain

- **Compilers:** gcc-13 and clang-18 are the supported set; gcc-9 is insufficient
  (no C++20 `= default operator==`).
- **CMake:** ≥ 3.21 (presets). Pip-installed cmake (`~/.local/bin/cmake`) auto-detected.
- **C++ standard:** C++20 throughout, `-Wall -Wextra -Wpedantic -Werror`
- **Sanitizers:** ASAN + UBSAN (`clang18-asan` preset), TSAN occasional (manual)
- **Coverage:** gcov-13 with the `coverage` preset, target ≥ 80% on changed lines
- **Static analysis:** clang-tidy with `.clang-tidy` config; cppcheck with `--enable=all`

```cmake
# Key CMake flags to know
-DQFAULT_ENABLE_ASAN=ON        # Address sanitizer build
-DQFAULT_ENABLE_UBSAN=ON       # UB sanitizer build
-DQFAULT_RUN_BENCHMARKS=ON     # Enables Google Benchmark targets
-DQFAULT_ENABLE_PYTHON=ON      # Builds pybind11 bindings (Stage 5)
-DQFAULT_GRIDSYNTH_PATH=<path> # Path to GridSynth binary/lib
```

## Build presets

| Preset | Compiler | Flavour | Purpose |
|--------|----------|---------|---------|
| `gcc13-debug` | gcc-13 | Debug | Default development |
| `gcc13-release` | gcc-13 | Release | Performance comparison |
| `clang18-debug` | clang-18 | Debug | Cross-compiler check |
| `clang18-release` | clang-18 | Release | Performance comparison |
| `clang18-asan` | clang-18 | RelWithDebInfo + sanitizers | ASAN + UBSAN gate |
| `coverage` | gcc-13 | Debug + gcov | Coverage gate |
| `gcc13-tsan` | gcc-13 | RelWithDebInfo + TSAN | Multithreading checks |
| `gcc13-perf` | gcc-13 | Release + `-pg` | Profiling |
| `system` | system default | Debug | Forbidden in CI; only for ad-hoc local |

## External dependencies (FetchContent)

| Dep | Version (pinned) | Purpose | Stage required |
|-----|------------------|---------|----------------|
| GoogleTest | 1.15.2 | Unit testing | 1 |
| Google Benchmark | v1.9.0 | Microbench harness | 2 |
| Stim | **v1.15.0** | Tableau equivalence oracle | 2.5 |
| MQT QCEC | **v3.5.0** | DD/ZX equivalence checking | 2.5 |
| pybind11 | v2.11.1 | Python bindings | 5b |
| tl::expected | v1.1.0 | C++23 `std::expected` shim | 3 |
| MPFR | 4.2 | Exact arithmetic for native Ross-Selinger | 6 |
| GMP | 6.3 | MPFR dependency | 6 |
| QIR | Alliance v0.1 base profile | QIR backend | 5a |

**External binaries** (host-installed, optional):
- GridSynth (Haskell, version-pinned in CI Dockerfile) — Stage 2 default synthesis

## Compile definitions

- `QFAULT_HAS_GRIDSYNTH` — set by `find_program(gridsynth)` if found
- `QFAULT_QIR_VERSION_MAJOR/MINOR/PATCH` — from `cmake/qir_version.cmake`
- `SIMD_WIDTH=64` — pinned for Stim golden reproducibility (ADR-0021)

## Files of interest

- `CMakeLists.txt` — root, presets, FetchContent
- `cmake/qir_version.cmake` — QIR pinned version (per ADR-0005)
- `.clang-tidy` — static analysis config
- `.clang-format` — style; **do not run blindly on existing files**, only on new ones
- `.gitattributes` — `text eol=lf` enforcement (per ADR-0017)
- `.github/workflows/ci.yml` — 4-way CI matrix + ASAN+UBSAN job

## Quick reference commands

```bash
# Build & test
cmake --preset gcc13-debug && cmake --build build/gcc13-debug -j && ctest --test-dir build/gcc13-debug -j

# Sanitizer pass
cmake --preset clang18-asan && cmake --build build/clang18-asan -j
ASAN_OPTIONS=detect_leaks=1 \
UBSAN_OPTIONS=print_stacktrace=1:halt_on_error=1 \
ctest --test-dir build/clang18-asan -j

# Coverage
cmake --preset coverage && cmake --build build/coverage -j
ctest --test-dir build/coverage -j
gcovr -r . build/coverage --html-details -o build/coverage/coverage.html

# Quick test (60s budget, runs before every commit)
./scripts/quick-test.sh

# Stage 3 — Stim equivalence diff (Stage 2.5+)
./scripts/compare-stim.sh tests/reference/bv10-d5.stim build/golden/

# Update goldens (intentional only — set the env var)
QFAULT_UPDATE_GOLDENS=1 ctest --test-dir build/gcc13-debug -R Golden
```
## Dependency-bump policy

Bumping any pinned version requires a new ADR with measured delta on the
relevant Stage gate test. Stim is expected to bump every 6–9 months; QCEC every
6–12 months; pybind11 stable. GridSynth (Haskell) is unlikely to need bumps.

## CI matrix (current)

```
job: build-and-test
  strategy:
    matrix:
      compiler: [gcc-13, clang-18]
      build-type: [Debug, Release]

job: sanitizers
  runs-on: ubuntu-24.04
  uses: clang18-asan preset

job: coverage
  runs-on: ubuntu-24.04
  uses: coverage preset
  reports: codecov.io

job: static-analysis
  runs-on: ubuntu-24.04
  steps: clang-tidy + cppcheck on changed files
```

For Stage 2.5+, add:

```
job: reproducibility
  runs-on: ubuntu-24.04
  steps:
    - docker build -f Dockerfile -t qfault:ci .
    - docker run qfault:ci make figures
    - diff bench/golden/expected/ bench/plots/
```
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
