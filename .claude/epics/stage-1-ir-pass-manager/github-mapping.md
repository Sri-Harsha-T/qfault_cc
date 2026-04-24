# GitHub Issue Mapping — Stage 1: IR + Pass Manager Core

Epic: #1 - https://github.com/Sri-Harsha-T/qfault_cc/issues/1
Milestone: #6 - Stage 1: IR + Pass Manager Core
Synced: 2026-04-24T18:51:00Z

## Tasks

| Local File | Issue # | Story ID | Title |
|------------|---------|----------|-------|
| 2.md | #2 | 1-D-1 | CMakeLists.txt with build presets |
| 3.md | #3 | 1-D-2 | GitHub Actions CI matrix |
| 4.md | #4 | 1-D-3 | clang-tidy + clang-format integration |
| 5.md | #5 | 1-D-4 | scripts/quick-test.sh (<60s) |
| 6.md | #6 | 1-A-1 | LogicalQubit, GateKind, LogicalGate types |
| 7.md | #7 | 1-A-2 | PatchCoord, MeasBasis, PatchOpKind, PatchOp types |
| 8.md | #8 | 1-A-3 | QFaultIRModule with two-level support |
| 9.md | #9 | 1-A-4 | module.dump(ostream&) IR printer |
| 10.md | #10 | 1-A-5 | Stage gate check — update ADR-0001 |
| 11.md | #11 | 1-B-1 | PassBase abstract class |
| 12.md | #12 | 1-B-2 | PassContext (code distance, diagnostics, timing) |
| 13.md | #13 | 1-B-3 | PassManager with add<T>() and run() |
| 14.md | #14 | 1-B-4 | NoOpPass + round-trip integration test |
| 15.md | #15 | 1-B-5 | PassManager::printStats(ostream) |
| 16.md | #16 | 1-C-1 | QASM 3.0 lexer (Clifford+T subset) |
| 17.md | #17 | 1-C-2 | QASM 3.0 parser → QFaultIRModule |
| 18.md | #18 | 1-C-3 | QASM 3.0 round-trip integration test |

## Dependency Graph (GitHub issue numbers)

```
#2 (1-D-1) ──┬──► #3 (1-D-2)
             ├──► #4 (1-D-3)  [parallel with #3, #5]
             ├──► #5 (1-D-4)  [parallel]
             ├──► #6 (1-A-1) ──┬──► #8 (1-A-3) ──┬──► #9 (1-A-4) ──► #10 (1-A-5)
             │                  │                  ├──► #11 (1-B-1) ──┐
             │                  │                  └──► #12 (1-B-2) ──┴──► #13 (1-B-3) ──┬──► #14 (1-B-4) ──► #18 (1-C-3)
             │                  │                                                          └──► #15 (1-B-5)
             └──► #7 (1-A-2) ──┘
             │
             └──► #6 (1-A-1) ──► #16 (1-C-1) ──► #17 (1-C-2) ──► #18 (1-C-3)
```

## First task to start: #2 (1-D-1)
