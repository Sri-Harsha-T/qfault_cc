# Stage 1 TODO — IR + Pass Manager Core

> This file is the daily inner loop tracker.
> **Status: COMPLETE** — all items shipped as of 2026-04-25.

## Epic 1-D: Build System (do first — unblocks everything)

- [x] 1-D-1: CMakeLists.txt with debug/release/asan/coverage presets (Issue #2, b262a63)
- [x] 1-D-2: GitHub Actions CI (clang-18 + gcc-13 matrix) (Issue #3, df0c6ce)
- [x] 1-D-3: .clang-tidy + .clang-format + make targets (Issue #4, 2503817)
- [x] 1-D-4: scripts/quick-test.sh (<60s) (Issue #5, 2503817)

## Epic 1-A: Core IR Data Structures

- [x] 1-A-1: LogicalQubit, GateKind, LogicalGate + unit tests (Issue #6, 2503817)
- [x] 1-A-2: PatchCoord, MeasBasis, PatchOpKind, PatchOp + unit tests (Issue #7, 2503817)
- [x] 1-A-3: QFaultIRModule (IRLevel, variant<LogicalGate,PatchOp>, assertLevel) (Issue #8, 30b4142)
- [x] 1-A-4: module.dump(ostream&) printer (Issue #9, 30b4142)
- [x] 1-A-5: Stage gate check — module holds both levels; ADR-0001 confirmed (Issue #10, 1b05ca4)

## Epic 1-B: PassManager

- [x] 1-B-1: PassBase abstract class (Issue #11, 30b4142)
- [x] 1-B-2: PassContext (d, synthesis config, logger, timing, diagnostics) (Issue #12, 30b4142)
- [x] 1-B-3: PassManager with add<T>() and run() + unit tests (Issue #13, dd025ac)
- [x] 1-B-4: NoOpPass + full round-trip integration test (Issue #14, fc8bdad)
- [x] 1-B-5: PassManager::printStats(ostream) (Issue #15, d081444)

## Epic 1-C: Frontend (QASM 3.0 subset)

- [x] 1-C-1: QASM 3.0 lexer (Clifford+T subset) (Issue #16, 6ef5bd4)
- [x] 1-C-2: QASM 3.0 parser → QFaultIRModule at LOGICAL level (Issue #17, 6ef5bd4)
- [x] 1-C-3: Round-trip integration test (parse → dump → parse) (Issue #18, 6ef5bd4)

## Stage Gate

- [x] ctest green: clang-18 debug + release
- [x] ctest green: gcc-13 debug + release
- [x] ASAN + UBSAN clean (UBSAN overflow in test_PassContext fixed in UBSAN fix commit)
- [x] clang-tidy clean
- [x] quick-test.sh < 60s
- [x] ADR-0001 updated with gate findings (Accepted)
- [x] activeContext.md "Next Action" set to Stage 2
- [x] progress.md Stage 1 row updated
- [x] CHANGELOG.md updated
- [x] GitHub Milestone "Stage 1" closed; all 18 issues Done
