# Stage 2 TODO — Synthesis Pass (T-Gate)

> Daily inner loop tracker for Stage 2.
> **Status: NOT STARTED** — begin after Stage 1 is fully closed.

## Pre-work: Finalise ADRs

- [ ] ADR-0002: SynthesisProvider via C++20 Concept → move to Accepted
- [ ] ADR-0003: Global code distance d for v0.1 → move to Accepted
- [ ] ADR-0004: GridSynth as default synthesiser → move to Accepted

## Epic 2-A: SynthesisProvider Concept + Providers

- [ ] 2-A-1: `SynthesisProvider` C++20 Concept + static_assert in test
- [ ] 2-A-2: `GridSynthProvider` (subprocess → parse stdout → std::vector<GateKind>)
- [ ] 2-A-3: `SKProvider` (Solovay-Kitaev pure C++, ε ≤ 1e-3)
- [ ] 2-A-4: Unit tests for both providers

## Epic 2-B: TGateSynthesisPass

- [ ] 2-B-1: `TGateSynthesisPass<Provider>` template class (satisfies PassConcept)
- [ ] 2-B-2: Integration test — T-gate replacement (no T/Tdg after pass)
- [ ] 2-B-3: T-count validation vs GridSynth reference tables (within 1%)

## Epic 2-C: CMake + CI Integration

- [ ] 2-C-1: `find_program(gridsynth)` + `QFAULT_HAS_GRIDSYNTH` cache var
- [ ] 2-C-2: `GTEST_SKIP()` in GridSynth-dependent tests when binary absent
- [ ] 2-C-3: `scripts/bench-synthesis.sh` + stage gate benchmark (≤5% overhead)

## Stage Gate

- [ ] ADRs 0002/0003/0004 Accepted
- [ ] ctest green: clang-18 debug + release
- [ ] ctest green: gcc-13 debug + release
- [ ] ASAN + UBSAN clean
- [ ] clang-tidy clean
- [ ] T-count validation within 1% of GridSynth reference
- [ ] Benchmark overhead ≤5% on 1000-gate circuit
- [ ] activeContext.md "Next Action" set to Stage 3
- [ ] progress.md Stage 2 row updated
- [ ] CHANGELOG.md updated
- [ ] GitHub Milestone "Stage 2" closed
