# Stage 2 Prompt Plan — Synthesis Pass (T-Gate)

Use these prompts in order with Claude Code. Each is one focused session.
Run `./scripts/quick-test.sh` at the start of every session to confirm the baseline.

---

## Pre-work: Verify ADR Status

```
/resume
Confirm: ADRs 0002, 0003, 0004 are all Accepted (they should be from Stage 1 close).
If any are still Draft, run /adr and finalize them before touching any code.
```

---

## Session 2-A-1: SynthesisProvider Concept

```
Issue #19 — 2-A-1: SynthesisProvider C++20 Concept

Define the SynthesisProvider concept in
  include/qfault/passes/synthesis/SynthesisProvider.hpp

The concept must require:
  { p.synthesise(angle, eps) } -> std::convertible_to<std::vector<GateKind>>
  { p.name() }                 -> std::convertible_to<std::string_view>

where `angle` is `double` (radians) and `eps` is `double` (synthesis precision).

Write a unit test file tests/unit/test_SynthesisProvider.cpp with:
  static_assert(SynthesisProvider<MockProvider>) using a trivial inline mock.
  static_assert(!SynthesisProvider<int>)

Build with gcc13-debug and clang18-debug. All 93 + new tests must be green.
```

---

## Session 2-A-2: GridSynthProvider

```
Issue #20 — 2-A-2: GridSynthProvider (subprocess wrapper)

Implement GridSynthProvider in:
  include/qfault/passes/synthesis/GridSynthProvider.hpp
  src/qfault/passes/synthesis/GridSynthProvider.cpp

Behaviour:
  - synthesise(angle, eps): invoke `gridsynth` binary via popen/subprocess
  - Parse the gate sequence from stdout (one gate per line or space-separated)
  - Return std::vector<GateKind>
  - If gridsynth binary not found (path empty), return empty vector and log
    DiagLevel::Warn via PassContext::addDiagnostic()
  - GridSynth binary path is read from compile-time definition GRIDSYNTH_BINARY
    (set by CMake in 2-C-1); if the definition is empty, skip gracefully

Unit test tests/unit/test_GridSynthProvider.cpp:
  - static_assert(SynthesisProvider<GridSynthProvider>)
  - If GRIDSYNTH_BINARY is not set: test calls GTEST_SKIP()
  - If available: synthesise(M_PI/4, 1e-10) returns a non-empty vector; all
    returned GateKind values are Clifford+T gates (H, S, T, X, Y, Z, Sdg, Tdg)

Build green on gcc13-debug and clang18-debug.
```

---

## Session 2-A-3: SKProvider (Solovay-Kitaev)

```
Issue #21 — 2-A-3: SKProvider (Solovay-Kitaev pure C++)

Implement Solovay-Kitaev in:
  include/qfault/passes/synthesis/SKProvider.hpp
  src/qfault/passes/synthesis/SKProvider.cpp

Requirements:
  - Depth-3 recursion (n=3 in the SK algorithm)
  - synthesise(angle, eps): returns a Clifford+T sequence approximating R_z(angle)
    with operator norm error ≤ eps (guarantee: eps ≤ 1e-3 for n=3)
  - No external dependencies — pure C++ implementation
  - name() returns "SKProvider"

The implementation should use the standard SK algorithm (Dawson & Nielsen 2005):
  - Basic approximation (lookup table of ~200 Clifford+T words)
  - Recursive depth-doubling until the error is within eps
  - Matrix representations of SU(2) used internally (not exported)

Unit test tests/unit/test_SKProvider.cpp:
  - static_assert(SynthesisProvider<SKProvider>)
  - Round-trip: synthesise(π/4, 1e-3) → verify the resulting unitary is within
    Frobenius norm 1e-3 of the target R_z(π/4)
  - Result contains only valid GateKind values (Clifford+T)

Build green on gcc13-debug and clang18-debug. ASAN + UBSAN clean.
```

---

## Session 2-A-4: Unit tests — both providers satisfy concept

```
Issue #22 — 2-A-4: Provider unit tests and concept static asserts

Add to tests/unit/test_SynthesisProvider.cpp:
  - static_assert(SynthesisProvider<GridSynthProvider>)
  - static_assert(SynthesisProvider<SKProvider>)
  - static_assert(!SynthesisProvider<int>)

Ensure test_GridSynthProvider.cpp and test_SKProvider.cpp both:
  - Have the right GTEST_SKIP() guards for GridSynth-binary-dependent tests
  - Are registered in CMakeLists.txt

Run: ctest -j on gcc13-debug and clang18-debug. All tests green.
```

---

## Session 2-B-1+2: TGateSynthesisPass + integration test

```
Issues #23 and #24 — 2-B-1 and 2-B-2

Implement TGateSynthesisPass<Provider> in:
  include/qfault/passes/synthesis/TGateSynthesisPass.hpp

Requirements:
  - Template parameter Provider must satisfy SynthesisProvider concept
  - requiredLevel() returns IRLevel::LOGICAL
  - run(QFaultIRModule& module, PassContext& ctx):
      - Calls module.assertLevel(IRLevel::LOGICAL)
      - Iterates the instruction vector
      - For each instruction that is a LogicalGate with kind T or Tdg:
          calls provider_.synthesise(angle, ctx.synthesisEpsilon())
          replaces the single T/Tdg with the returned gate sequence
      - All other instructions pass through unchanged
  - Accumulates stat: count of T-gates replaced

Integration test tests/integration/test_synthesis_roundtrip.cpp:
  - Construct a QFaultIRModule with 10 instructions (3 T-gates, rest Clifford)
    at LOGICAL level (inline construction, no parser dependency)
  - Run PassManager with TGateSynthesisPass<SKProvider>
  - Assert: (a) no GateKind::T or GateKind::Tdg remain in output
  - Assert: (b) instruction count > 10 (T-gates expanded)
  - Assert: (c) all remaining gate kinds are valid Clifford+T values
  - Assert: (d) non-T instructions are identical to input (positions preserved)

Build green on gcc13-debug and clang18-debug.
```

---

## Session 2-B-3: T-count validation vs GridSynth reference

```
Issue #25 — 2-B-3: T-count validation vs GridSynth reference table

Write test tests/unit/test_TCountValidation.cpp:

For 5 standard angles (π/4, π/8, π/16, π/32, 3π/8):
  1. Run GridSynthProvider::synthesise(angle, 1e-10)
  2. Count the number of T and Tdg gates in the result
  3. Compare against the reference T-count from Ross & Selinger 2016, Table 1

Acceptance criteria:
  - For each angle, |qfault_tcount - reference_tcount| / reference_tcount ≤ 0.01 (1%)
  - Test skips if GridSynth binary is not available (GTEST_SKIP())

This test is the primary evidence for the Stage 2 gate.
```

---

## Session 2-C-1+2: CMake optional dependency + skip guards

```
Issues #26 and #27 — 2-C-1 and 2-C-2

In CMakeLists.txt, add after the existing targets:

  find_program(GRIDSYNTH_BINARY gridsynth)
  if(GRIDSYNTH_BINARY)
    message(STATUS "GridSynth found: ${GRIDSYNTH_BINARY}")
    set(QFAULT_HAS_GRIDSYNTH ON CACHE BOOL "GridSynth binary found" FORCE)
    target_compile_definitions(qfault_tests PRIVATE
      GRIDSYNTH_BINARY="${GRIDSYNTH_BINARY}"
      QFAULT_HAS_GRIDSYNTH=1)
  else()
    message(WARNING "GridSynth not found. GridSynth-dependent tests will be skipped.")
    message(WARNING "Falling back to Solovay-Kitaev (not T-optimal) for production use.")
  endif()

Verify:
  - ctest passes on a machine without gridsynth (all GridSynth tests show SKIPPED)
  - No test shows FAILED when gridsynth is absent
  - The CI workflow (.github/workflows/ci.yml) does NOT install gridsynth
    (so the skip logic is exercised on every CI run)
```

---

## Session 2-C-3: Benchmark script + stage gate

```
Issue #28 — 2-C-3: bench-synthesis.sh + overhead ≤5% stage gate

Write scripts/bench-synthesis.sh:
  - Generates a 1000-gate Clifford+T circuit with 300 T-gates (inline QASM 3.0)
  - Runs two timings:
      (a) Direct GridSynth binary calls for each T-gate (baseline)
      (b) QFault TGateSynthesisPass<GridSynthProvider> on the same circuit
  - Reports overhead: (b-a)/a as a percentage
  - Exits 1 if overhead > 5%

This script is the stage gate check. Run it as the final step before closing Stage 2.
Prerequisite: GridSynth binary must be installed.
```

---

## Session GATE: Stage 2 Close

```
/phase-exit (or manual gate check)

1. Run ./scripts/quick-test.sh — must be 100% green
2. Run cmake --preset clang18-asan && cmake --build build/clang18-asan -j
   ctest --test-dir build/clang18-asan — ASAN+UBSAN clean
3. Run scripts/bench-synthesis.sh — overhead ≤5%
4. Verify T-count validation test passes with GridSynth installed
5. Update docs/phases/stage-2-synthesis/spec.md Definition of Done (check all boxes)
6. Write docs/phases/stage-2-synthesis/exit-report.md
7. Update memory-bank/activeContext.md with Stage 3 "Next Action"
8. Update memory-bank/progress.md Stage 2 row
9. Update CHANGELOG.md
10. Close GitHub Milestone "Stage 2: Synthesis Pass"
```
