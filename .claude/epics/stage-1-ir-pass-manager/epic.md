---
name: stage-1-ir-pass-manager
status: in-progress
created: 2026-04-24T18:42:05Z
updated: 2026-04-24T18:50:57Z
progress: 0%
prd: docs/phases/stage-1-ir-pass-manager/spec.md
github: https://github.com/Sri-Harsha-T/qfault_cc/issues/1
---

# Epic: Stage 1 — IR + Pass Manager Core

## Goal

Build the foundational `QFaultIR` data structures and `PassManager` that all subsequent compiler passes will operate over. No synthesis or QEC-specific logic yet — this is the pure infrastructure layer.

## Stage Gate

> The IR can cleanly represent both logical Clifford+T gates AND surface code patch operations in the same data structure, without a lossy conversion. A no-op pass chain ingests a simple Clifford+T circuit and produces identical output, with a full test harness passing.

## Execution Order

Work proceeds in this order — **Epic 1-D (build system) must complete first** as it unblocks all code work:

1. **1-D** (Build System) — CMakeLists.txt → CI → linting → quick-test.sh
2. **1-A** (IR Types) — LogicalGate/PatchOp types → QFaultIRModule → dump printer → stage gate check
3. **1-B** (PassManager) — PassBase → PassContext → PassManager → NoOpPass → printStats
4. **1-C** (Frontend) — QASM 3.0 lexer → parser → round-trip integration test

## Sub-Epics

### Epic 1-D: CMake Build System

| Story | Description | Est. |
|-------|-------------|------|
| 1-D-1 | CMakeLists.txt with debug/release/asan/coverage presets | 2d |
| 1-D-2 | GitHub Actions CI: clang-17 + gcc-13 matrix | 1d |
| 1-D-3 | .clang-tidy + .clang-format + make tidy/format targets | 1d |
| 1-D-4 | scripts/quick-test.sh (completes in <60s) | 0.5d |

### Epic 1-A: Core IR Data Structures

| Story | Description | Est. |
|-------|-------------|------|
| 1-A-1 | LogicalQubit, GateKind (enum class), LogicalGate + unit tests | 2d |
| 1-A-2 | PatchCoord, MeasBasis, PatchOpKind, PatchOp + unit tests | 2d |
| 1-A-3 | QFaultIRModule: IRLevel tag, variant<LogicalGate,PatchOp>, assertLevel() | 2d |
| 1-A-4 | module.dump(ostream&) — human-readable IR printer | 1d |
| 1-A-5 | Stage gate check: confirm module holds both levels; update ADR-0001 | 1d |

### Epic 1-B: PassManager

| Story | Description | Est. |
|-------|-------------|------|
| 1-B-1 | PassBase abstract class: name(), requiredLevel(), run(module, ctx) | 1d |
| 1-B-2 | PassContext: code distance d, synthesis config, logger, timing, diagnostics | 2d |
| 1-B-3 | PassManager: add<T>() and run() with diagnostics collection | 2d |
| 1-B-4 | NoOpPass + full round-trip integration test | 1d |
| 1-B-5 | PassManager::printStats(ostream) — per-pass timing + instruction count | 1d |

### Epic 1-C: Frontend — QASM 3.0 Parser (subset)

| Story | Description | Est. |
|-------|-------------|------|
| 1-C-1 | QASM 3.0 lexer: tokenises qubit, gate, h, cx, t, s, tdg, sdg, identifiers, semicolons | 3d |
| 1-C-2 | QASM 3.0 parser → QFaultIRModule at LOGICAL level | 3d |
| 1-C-3 | Integration test: parse → dump → parse produces identical module | 1d |

## Definition of Done

- [ ] All story ACs pass (ctest -j green on clang-17 AND gcc-13, debug AND release)
- [ ] ASAN + UBSAN clean on all test targets
- [ ] clang-tidy clean on all src/ files
- [ ] ./scripts/quick-test.sh completes in <60s
- [ ] Stage gate documented: ADR-0001 updated with findings
- [ ] memory-bank/activeContext.md updated with Stage 2 "Next Action"
- [ ] memory-bank/progress.md Stage 1 row marked complete
- [ ] CHANGELOG.md updated
- [ ] GitHub Milestone "Stage 1: IR + Pass Manager Core" closed

## Tasks Created

- [ ] 001.md - 1-D-1: CMakeLists.txt with build presets (parallel: false)
- [ ] 002.md - 1-D-2: GitHub Actions CI matrix (parallel: false, depends: 001)
- [ ] 003.md - 1-D-3: clang-tidy + clang-format integration (parallel: true, depends: 001)
- [ ] 004.md - 1-D-4: scripts/quick-test.sh (parallel: true, depends: 001)
- [ ] 005.md - 1-A-1: LogicalQubit, GateKind, LogicalGate types (parallel: true, depends: 001)
- [ ] 006.md - 1-A-2: PatchCoord, MeasBasis, PatchOpKind, PatchOp types (parallel: true, depends: 001)
- [ ] 007.md - 1-A-3: QFaultIRModule with two-level support (parallel: false, depends: 005,006)
- [ ] 008.md - 1-A-4: module.dump(ostream&) IR printer (parallel: false, depends: 007)
- [ ] 009.md - 1-A-5: Stage gate check — update ADR-0001 (parallel: false, depends: 007,008)
- [ ] 010.md - 1-B-1: PassBase abstract class (parallel: true, depends: 007)
- [ ] 011.md - 1-B-2: PassContext class (parallel: true, depends: 007)
- [ ] 012.md - 1-B-3: PassManager with add<T>() and run() (parallel: false, depends: 010,011)
- [ ] 013.md - 1-B-4: NoOpPass + round-trip integration test (parallel: false, depends: 012,008)
- [ ] 014.md - 1-B-5: PassManager::printStats(ostream) (parallel: false, depends: 012)
- [ ] 015.md - 1-C-1: QASM 3.0 lexer (parallel: false, depends: 005)
- [ ] 016.md - 1-C-2: QASM 3.0 parser → QFaultIRModule (parallel: false, depends: 015,007)
- [ ] 017.md - 1-C-3: QASM 3.0 round-trip integration test (parallel: false, depends: 016,008,013)

Total tasks: 17
Parallel tasks: 6 (003, 004, 005, 006, 010, 011)
Sequential tasks: 11
Estimated total effort: ~42 hours (~3.5 weeks solo)
