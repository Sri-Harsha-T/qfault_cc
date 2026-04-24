# Stage 1 Spec: IR + Pass Manager Core

**Stage:** 1 of 5  
**Target duration:** 3–4 weeks  
**GitHub Milestone:** "Stage 1: IR + Pass Manager Core"

---

## Goal

Build the foundational `QFaultIR` data structures and `PassManager` that all
subsequent compiler passes will operate over. No synthesis or QEC-specific
logic yet — this is the pure infrastructure layer.

**Stage Gate (hard stop before Stage 2):**
> The IR can cleanly represent both logical Clifford+T gates AND surface code
> patch operations in the same data structure, without a lossy conversion.
> A no-op pass chain ingests a simple Clifford+T circuit and produces
> identical output, with a full test harness passing.

---

## Epics and Stories

### Epic 1-A: Core IR Data Structures

**Goal:** Define `QFaultIR` types that can represent both circuit levels.

| Story | Acceptance Criteria | Est. |
|-------|---------------------|------|
| 1-A-1: Define `LogicalGate` and `LogicalQubit` types | Struct definitions with Doxygen; `==` operators; unit tests for construction | 2d |
| 1-A-2: Define `PatchCoord`, `PatchOp`, `MeasBasis` types | Struct definitions; `==` operators; unit tests | 2d |
| 1-A-3: Define `QFaultIRModule` with two-level support | `std::variant<LogicalGate, PatchOp>` instruction vector; `IRLevel` tag; `assertLevel()` method | 2d |
| 1-A-4: IR printer (text dump) | `module.dump(std::ostream&)` produces human-readable output; round-trip parse test not required yet | 1d |
| 1-A-5: **Stage gate check** | Answer: Can the same module cleanly hold both levels? Document findings in ADR-0001 update. | 1d |

### Epic 1-B: PassManager

**Goal:** A composable pass runner that chains passes over `QFaultIRModule`.

| Story | Acceptance Criteria | Est. |
|-------|---------------------|------|
| 1-B-1: `PassBase` abstract class | `name()`, `requiredLevel()`, `run(module, ctx)` interface | 1d |
| 1-B-2: `PassContext` class | Holds: code distance d, synthesis config, logger, timing, diagnostics | 2d |
| 1-B-3: `PassManager` with `add<T>()` and `run()` | Type-safe add; ordered pass execution; collects diagnostics | 2d |
| 1-B-4: `NoOpPass` (validates the framework) | Pass that reads all instructions and writes them unchanged; full round-trip test | 1d |
| 1-B-5: Pass timing and diagnostics output | `PassManager::printStats(ostream)` shows per-pass time and instruction count | 1d |

### Epic 1-C: Frontend (minimal QASM 3.0 parser)

**Goal:** Read a simple Clifford+T circuit from QASM 3.0 text into `QFaultIRModule`.

| Story | Acceptance Criteria | Est. |
|-------|---------------------|------|
| 1-C-1: QASM 3.0 lexer (subset) | Tokenises: `qubit`, `gate`, `h`, `cx`, `t`, `s`, `tdg`, `sdg`, identifiers, semicolons | 3d |
| 1-C-2: QASM 3.0 parser → LogicalGates | Parses gate applications; populates `QFaultIRModule` at `LOGICAL` level | 3d |
| 1-C-3: Integration test: QASM 3.0 round-trip | Parse → dump → parse produces identical module | 1d |

### Epic 1-D: CMake Build System

| Story | Acceptance Criteria | Est. |
|-------|---------------------|------|
| 1-D-1: CMakeLists.txt with presets | `debug`, `release`, `asan`, `coverage` presets; GoogleTest via FetchContent | 2d |
| 1-D-2: CI GitHub Actions | Matrix: {clang-17, gcc-13} × {debug, release}; ctest runs on push | 1d |
| 1-D-3: clang-tidy + clang-format integration | `make tidy` and `make format` targets; `.clang-tidy` and `.clang-format` config | 1d |
| 1-D-4: `scripts/quick-test.sh` | Runs ctest in < 60 seconds; used as pre-commit check | 0.5d |

---

## Stage 1 Definition of Done

- [ ] All story ACs pass (`ctest -j` green on clang-17 AND gcc-13 in debug and release)
- [ ] ASAN + UBSAN clean on all test targets
- [ ] `clang-tidy` clean on all `src/` files
- [ ] `./scripts/quick-test.sh` completes in < 60 seconds
- [ ] Stage gate documented: ADR-0001 updated with findings
- [ ] `memory-bank/activeContext.md` updated with Stage 2 "Next Action"
- [ ] `memory-bank/progress.md` Stage 1 row marked complete with date
- [ ] `CHANGELOG.md` updated with any failed approaches found during Stage 1
- [ ] GitHub Milestone "Stage 1" closed; all issues Done

---

## Prompt Plan for Claude Code Sessions

Use these in order with Claude Code. Each is one focused session.

```
Session 1-D-1: "Set up CMakeLists.txt for QFault. 
  Requirements: C++20, CMake ≥3.20, GoogleTest via FetchContent, 
  presets: debug/release/asan/coverage. Target: libqfault + test binary.
  Follow memory-bank/techContext.md for toolchain details."

Session 1-A-1+2: "Define QFaultIR core types in src/qfault/ir/. 
  Types needed: LogicalQubit, GateKind (enum class), LogicalGate, 
  PatchCoord, MeasBasis, PatchOpKind, PatchOp.
  Follow ADR-0001 for variant design. Write unit tests."

Session 1-A-3+4: "Define QFaultIRModule with IRLevel tag and instruction 
  variant. Add assertLevel(), dump(ostream&). Write unit tests for 
  both LOGICAL and PHYSICAL level modules."

Session 1-B-1+2+3: "Implement PassBase, PassContext, and PassManager.
  Follow memory-bank/systemPatterns.md for design. Unit test the 
  pass ordering and diagnostic collection."

Session 1-B-4+5: "Implement NoOpPass and run full round-trip test.
  Add PassManager::printStats(). Integration test: parse simple 
  Clifford+T circuit → NoOpPass → compare output."

Session 1-C-1+2+3: "Implement minimal QASM 3.0 lexer + parser for 
  Clifford+T gates. Input: QASM 3.0 string. Output: QFaultIRModule 
  at LOGICAL level. Round-trip integration test required."

Session 1-D-2+3+4: "Set up GitHub Actions CI matrix, clang-tidy config, 
  and scripts/quick-test.sh."

Session GATE: "Run stage gate check per docs/phases/stage-1/spec.md.
  Run all tests, asan-ubsan, clang-tidy. Update ADR-0001, 
  activeContext.md, progress.md, CHANGELOG.md."
```
