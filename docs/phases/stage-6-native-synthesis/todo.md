# Stage 6 TODO — Native Synthesis

> Daily inner-loop tracker for Stage 6 (optional / contingent).
> Status: planning complete; kickoff pending Stage 5 closure.

## Pre-work: confirm Stage 5 closed

- [ ] Stage 5a/5b/5c exit reports written.
- [ ] v0.1.0 release tagged.
- [ ] Zenodo DOI minted for v0.1.0.
- [ ] QCE26 submission frozen at `papers/qce-2026/`.
- [ ] ADR-0008 reviewed for any Stage 6 scope changes from feedback.

## Epic 6-A: MPFR + GMP wrappers

- [ ] 6-A-1: `cmake/mpfr_config.cmake`, MPFR ≥ 4.2.
- [ ] 6-A-2: `cmake/gmp_config.cmake`, GMP ≥ 6.3.
- [ ] 6-A-3: `qfault::math::Real`, `qfault::math::Integer` wrappers.
- [ ] 6-A-4: Wrapper unit tests; ASAN+UBSAN clean.

## Epic 6-D: SKProvider → BFSTableProvider rename (do early)

- [ ] 6-D-1: Move source/header files, rename class.
- [ ] 6-D-2: Deprecated `SKProvider` alias header.
- [ ] 6-D-3: Test file rename and call-site updates.
- [ ] 6-D-4: README + ADR-0013 status update.

## Epic 6-B: NativeRSProvider

- [ ] 6-B-1: Scaffold + Concept satisfaction.
- [ ] 6-B-2: `qfault::math::Z_omega` for ℤ[ω].
- [ ] 6-B-3: Grid-problem solver (Ross-Selinger §5).
- [ ] 6-B-4: Diophantine factorization (Ross-Selinger §6).
- [ ] 6-B-5: T-count parity test vs `GridSynthProvider` within 1%.
- [ ] 6-B-6: Throughput benchmark — target ≥ 5× faster than gridsynth.

## Epic 6-C: KliuchnikovProvider

- [ ] 6-C-1: Scaffold + Concept satisfaction.
- [ ] 6-C-2: Mixed-fallback core (Kliuchnikov 2023 §3).
- [ ] 6-C-3: RUS protocol wrapper.
- [ ] 6-C-4: T-count parity test vs Qualtran within 5%.

## Epic 6-E: Concept-boundary regression

- [ ] 6-E-1: Six positive `static_assert`s in test_SynthesisProvider.cpp.
- [ ] 6-E-2: Parameterized `test_synthesis_roundtrip.cpp` over four
            providers.
- [ ] 6-E-3: README synthesis section refreshed.

## Stage Gate

- [ ] ctest green across all four providers on gcc-13 + clang-18.
- [ ] ASAN + UBSAN clean (especially MPFR/GMP-touching code).
- [ ] clang-tidy clean.
- [ ] Throughput target met: `NativeRSProvider` ≥ 5× faster than gridsynth.
- [ ] T-count parity met: Ross-Selinger within 1%, Kliuchnikov within 5%.
- [ ] activeContext.md "Next Action" set to Stage 7 selection.
- [ ] progress.md Stage 6 row updated.
- [ ] CHANGELOG.md updated.
- [ ] GitHub Milestone "Stage 6: Native Synthesis" closed.
- [ ] exit-report.md written.
