# Stage 1 TODO — IR + Pass Manager Core

> This file is the daily inner loop tracker. Claude Code updates it via TodoWrite.
> GitHub Issues track the same items at epic/story granularity.

## Epic 1-D: Build System (do first — unblocks everything)

- [ ] 1-D-1: CMakeLists.txt with debug/release/asan/coverage presets
- [ ] 1-D-2: GitHub Actions CI (clang-17 + gcc-13 matrix)
- [ ] 1-D-3: .clang-tidy + .clang-format + make targets
- [ ] 1-D-4: scripts/quick-test.sh (<60s)

## Epic 1-A: Core IR Data Structures

- [ ] 1-A-1: LogicalQubit, GateKind, LogicalGate + unit tests
- [ ] 1-A-2: PatchCoord, MeasBasis, PatchOpKind, PatchOp + unit tests
- [ ] 1-A-3: QFaultIRModule (IRLevel, variant<LogicalGate,PatchOp>, assertLevel)
- [ ] 1-A-4: module.dump(ostream&) printer
- [ ] 1-A-5: Stage gate check — can module hold both levels? Update ADR-0001

## Epic 1-B: PassManager

- [ ] 1-B-1: PassBase abstract class
- [ ] 1-B-2: PassContext (d, synthesis config, logger, timing, diagnostics)
- [ ] 1-B-3: PassManager with add<T>() and run() + unit tests
- [ ] 1-B-4: NoOpPass + full round-trip integration test
- [ ] 1-B-5: PassManager::printStats(ostream)

## Epic 1-C: Frontend (QASM 3.0 subset)

- [ ] 1-C-1: QASM 3.0 lexer (Clifford+T subset)
- [ ] 1-C-2: QASM 3.0 parser → QFaultIRModule at LOGICAL level
- [ ] 1-C-3: Round-trip integration test (parse → dump → parse)

## Stage Gate

- [ ] ctest green: clang-17 debug + release
- [ ] ctest green: gcc-13 debug + release
- [ ] ASAN + UBSAN clean
- [ ] clang-tidy clean
- [ ] quick-test.sh < 60s
- [ ] ADR-0001 updated with gate findings
- [ ] activeContext.md "Next Action" set to Stage 2
- [ ] progress.md Stage 1 row updated
- [ ] CHANGELOG.md updated
- [ ] GitHub Milestone "Stage 1" closed
