# Stage 2 TODO — Synthesis Pass (T-Gate)

> Daily inner loop tracker for Stage 2.
> **Status: CODE COMPLETE (2026-04-25)** — gate pending GridSynth installation.

## Pre-work: Finalise ADRs

- [x] ADR-0002: SynthesisProvider via C++20 Concept → Accepted (was already Accepted from Stage 1)
- [x] ADR-0003: Global code distance d for v0.1 → Accepted
- [x] ADR-0004: GridSynth as default synthesiser → Accepted

## Epic 2-A: SynthesisProvider Concept + Providers

- [x] 2-A-1: `SynthesisProvider` C++20 Concept + static_assert in test (#19)
- [x] 2-A-2: `GridSynthProvider` (subprocess → parse stdout → std::vector<GateKind>) (#20)
- [x] 2-A-3: `SKProvider` (Solovay-Kitaev pure C++, depth-7 BFS basic set) (#21)
- [x] 2-A-4: Unit tests for both providers + concept static_asserts (#22)

## Epic 2-B: TGateSynthesisPass

- [x] 2-B-1: `TGateSynthesisPass<Provider>` template class (satisfies PassConcept) (#23)
- [x] 2-B-2: Integration test — T-gate replacement (no T/Tdg after CliffordOnlyProvider) (#24)
- [x] 2-B-3: T-count validation test (GTEST_SKIP when GridSynth absent) (#25)

## Epic 2-C: CMake + CI Integration

- [x] 2-C-1: `find_program(gridsynth)` + `QFAULT_HAS_GRIDSYNTH` cache var (#26)
- [x] 2-C-2: `GTEST_SKIP()` in GridSynth-dependent tests when binary absent (#27)
- [x] 2-C-3: `scripts/bench-synthesis.sh` stage gate benchmark script (#28)

## Stage Gate

- [x] ADRs 0002/0003/0004 Accepted
- [x] ctest green: gcc-13 debug
- [x] ctest green: clang-18 debug
- [x] ASAN + UBSAN clean (clang18-asan)
- [ ] clang-tidy clean (clang-tidy-18 not installed on dev machine)
- [ ] T-count validation within 1% of GridSynth reference (requires GridSynth binary)
- [ ] Benchmark overhead ≤5% on 1000-gate circuit (requires GridSynth binary)
- [ ] activeContext.md "Next Action" set to Stage 3
- [ ] progress.md Stage 2 row updated (marked complete with date)
- [ ] CHANGELOG.md updated
- [ ] GitHub Milestone "Stage 2" closed
- [ ] exit-report.md written
