# Stage 1 Kickoff — IR + Pass Manager Core

**Date:** 2026-04-24
**Stage:** 1 of 5
**Milestone:** "Stage 1: IR + Pass Manager Core"

---

## Objectives

Build the foundational `QFaultIR` data structures and `PassManager` that all
subsequent compiler passes will operate over. The stage produces:

1. A two-level IR capable of representing both logical Clifford+T gates and
   surface code physical patch operations in a single data structure
2. A composable pass manager that chains compiler passes and collects diagnostics
3. A minimal QASM 3.0 frontend for reading input circuits
4. A complete build system (CMake + CI) that all subsequent stages build on

**Non-goals for Stage 1:**
- No actual synthesis (GridSynth integration is Stage 2)
- No lattice surgery (Stage 3)
- No resource estimation (Stage 4)
- No output backends other than the IR printer (Stage 5)

---

## Key Design Questions Going In

1. **Two-level IR:** Use a single `QFaultIRModule` with `variant<LogicalGate, PatchOp>`,
   or two separate module types? → ADR-0001 proposes single module; this stage validates it.

2. **Pass interface:** Virtual dispatch (`PassBase*`) or C++20 Concepts? → ADR-0004 proposes
   Concepts; this stage implements and tests the Concept approach.

3. **Compiler support:** C++20 throughout. Target: clang-18 and gcc-13 with `-Werror`.

---

## Execution Order

Epic 1-D (build system) first — it unblocks everything else.

```
1-D-1 CMakeLists.txt    ──► 1-D-3 clang-tidy     ──► all other epics
1-D-2 CI                    1-D-4 quick-test.sh
                                  │
1-A-1 LogicalGate    ──► 1-A-3 IRModule ──► 1-A-5 Stage gate
1-A-2 PatchOp                          └──► 1-B-1 PassBase
                                            1-B-2 PassContext
                                            1-B-3 PassManager ──► 1-B-4 NoOpPass ──► 1-B-5 printStats
                                                                        │
                                                               1-C-1 Lexer ──► 1-C-2 Parser ──► 1-C-3 Round-trip
```

---

## Tools and Dependencies

| Tool | Version | Source |
|------|---------|--------|
| CMake | ≥3.21 | pip (`~/.local/bin/cmake`) |
| clang++ | 18 | apt `clang-18` |
| g++ | 13 | apt `g++-13` |
| clang-tidy | 18 | apt `clang-tidy-18` |
| GoogleTest | latest | FetchContent (no system install needed) |

---

## Success Criteria (from spec.md)

Stage gate hard stop before Stage 2:
> The IR can cleanly represent both logical Clifford+T gates AND surface code
> patch operations in the same data structure, without a lossy conversion.
> A no-op pass chain ingests a simple Clifford+T circuit and produces
> identical output, with a full test harness passing.
