# Stage 6 Spec: Native Synthesis — C++ Ross-Selinger + Kliuchnikov-2023

**Stage:** 6 of 7 (optional / contingent)
**Target duration:** 12–20 weeks
**GitHub Milestone:** "Stage 6: Native Synthesis"
**Depends on:** Stage 5 (release) complete

---

## Goal

Close two gaps that the v0.1 audit identified:

1. **C++ throughput on synthesis** — replace `GridSynthProvider`'s `popen`
   subprocess wrapper with a native C++ Ross-Selinger implementation.
2. **Algorithm-portfolio parity with Qualtran** — add Kliuchnikov-2023
   mixed-fallback / repeat-until-success as a second native provider.

Both providers slot in behind the existing `SynthesisProvider` Concept
(per ADR-0002) with **zero refactoring** at the `TGateSynthesisPass` call
site. This is the stage that makes the C++-throughput pitch defensible.

**Stage Gate (hard stop before Stage 7 selection):**

> The native C++ Ross-Selinger provider achieves ≥ 5× throughput over the
> `gridsynth` subprocess wrapper at matched T-count and ε = 10⁻¹⁰, on a
> corpus of 10⁴ random rotations. T-count parity with the Haskell
> reference is within 1% on the Ross-Selinger 2016 Table 1 angles. The
> Kliuchnikov provider matches Qualtran's published T-counts within 5%
> on a 100-rotation regression corpus.

---

## Why this stage exists (rationale)

The v0.1 audit (per ADR-0008) explicitly retracted the "C++ throughput on
synthesis" claim because `GridSynthProvider` shells out to Haskell. The
audit also noted that Qualtran is the synthesis-portfolio frontier with
both Ross-Selinger and Kliuchnikov-2023 in production. Stage 6 is the
designated catch-up.

Stage 6 is **optional / contingent on collaborator availability**. If
Stage 5c (the QCE26 submission) runs longer than planned, Stage 6 slips
to the v0.2 milestone without invalidating Stage 5's release.

---

## ADRs to consult before kickoff

| ADR | Topic |
|-----|-------|
| ADR-0002 | SynthesisProvider Concept — the boundary new providers slot into |
| ADR-0004 | GridSynth as v0.1 default (limitation note) |
| ADR-0008 | Synthesis algorithm portfolio (declares this stage's contents) |
| ADR-0013 | SKProvider rename to BFSTableProvider (preserve as alias) |

ADRs 0008 and 0013 are the two most important. If Stage 6 introduces a
fundamentally different provider interface, a new ADR is required.

---

## Epics and Stories

### Epic 6-A: MPFR + GMP integration

| Story | Acceptance Criteria | Est. |
|-------|---------------------|------|
| 6-A-1: MPFR FetchContent / find_package | `cmake/mpfr_config.cmake` declares MPFR ≥ 4.2; falls back to system find_package on Linux package managers. Builds on gcc-13 and clang-18. | 2d |
| 6-A-2: GMP FetchContent / find_package | `cmake/gmp_config.cmake` declares GMP ≥ 6.3; same dual-path discovery. | 2d |
| 6-A-3: Wrapper namespace | `qfault::math::Real` and `qfault::math::Integer` thin wrappers in `include/qfault/math/`. Hide raw `mpfr_t` / `mpz_t` from the rest of the codebase. | 3d |
| 6-A-4: Wrapper unit tests | `tests/unit/test_Real.cpp` and `test_Integer.cpp` cover construction, arithmetic, comparison, and conversion to `double`. ASAN+UBSAN clean. | 2d |

### Epic 6-B: Native C++ Ross-Selinger

| Story | Acceptance Criteria | Est. |
|-------|---------------------|------|
| 6-B-1: NativeRSProvider scaffold | `include/qfault/passes/synthesis/NativeRSProvider.hpp` and `src/.../NativeRSProvider.cpp`. Satisfies `SynthesisProvider` Concept (`static_assert` in test file). | 2d |
| 6-B-2: Ring-element representation | `qfault::math::Z_omega` for elements of ℤ[ω] (ω = e^{iπ/4}); supports add, multiply, conjugate, norm. | 4d |
| 6-B-3: Grid-problem solver | Implementation of Ross-Selinger §5 (the grid problem and ε-region search). Returns the optimal Clifford+T sequence. | 6d |
| 6-B-4: Diophantine factorization | Ross-Selinger §6 (the Diophantine equation `t·t* = u`). MPFR-validated. | 5d |
| 6-B-5: T-count parity test | `tests/unit/test_NativeRSProvider.cpp` checks T-count within 1% of `GridSynthProvider` on Ross-Selinger 2016 Table 1 angles at ε = 10⁻¹⁰. | 2d |
| 6-B-6: Throughput benchmark | `bench/tier1/native_rs_vs_gridsynth.sh` measures synthesis throughput on a 10⁴ random angle corpus. Target: ≥ 5× speedup. | 2d |

### Epic 6-C: Kliuchnikov-2023 provider

| Story | Acceptance Criteria | Est. |
|-------|---------------------|------|
| 6-C-1: KliuchnikovProvider scaffold | `include/qfault/passes/synthesis/KliuchnikovProvider.hpp` etc. Concept satisfaction. | 2d |
| 6-C-2: Mixed-fallback core | Kliuchnikov 2023 §3 mixed-fallback synthesis. Returns either an exact decomposition or a fallback list with measurement-conditional gates. | 8d |
| 6-C-3: RUS protocol | Repeat-until-success wrapper that consumes the fallback list. Handles measurement and re-attempt logic at the IR level (not at the gate level). | 5d |
| 6-C-4: T-count parity vs Qualtran | `tests/unit/test_KliuchnikovProvider.cpp` checks T-count parity within 5% of Qualtran's `rotation_synthesis` module on a 100-rotation regression corpus. | 4d |

### Epic 6-D: SKProvider rename to BFSTableProvider

| Story | Acceptance Criteria | Est. |
|-------|---------------------|------|
| 6-D-1: Rename source files | Move `SKProvider.{hpp,cpp}` → `BFSTableProvider.{hpp,cpp}`. Class renamed throughout. | 1d |
| 6-D-2: Deprecated alias | `include/qfault/passes/synthesis/SKProvider.hpp` becomes a thin alias header: `using SKProvider [[deprecated(...)]] = BFSTableProvider;`. Compiles with deprecation warning. | 1d |
| 6-D-3: Test updates | `tests/unit/test_SKProvider.cpp` renamed to `test_BFSTableProvider.cpp`. All call sites updated. | 1d |
| 6-D-4: README and ADR updates | README's synthesis section reflects the rename. ADR-0013 transitions from "Accepted" to "Implemented". | 1d |

### Epic 6-E: Concept-boundary regression

| Story | Acceptance Criteria | Est. |
|-------|---------------------|------|
| 6-E-1: Six static_asserts | `tests/unit/test_SynthesisProvider.cpp` extended with positive `static_assert`s for `NativeRSProvider` and `KliuchnikovProvider`. Negative `static_assert`s already present. | 1d |
| 6-E-2: TGateSynthesisPass parity | `tests/integration/test_synthesis_roundtrip.cpp` parameterized over all four providers (`GridSynthProvider`, `BFSTableProvider`, `NativeRSProvider`, `KliuchnikovProvider`); each must satisfy the same instruction-count expansion bound. | 2d |
| 6-E-3: README synthesis-section refresh | README's synthesis section enumerates all four providers, names their algorithms, and explicitly cites the audit-driven gap closure. | 1d |

---

## Stage 6 Definition of Done

- [ ] All story ACs pass (`ctest -j` green on clang-18 AND gcc-13 in
      debug AND release).
- [ ] ASAN + UBSAN clean on all native synthesis tests (especially
      MPFR/GMP-touching code; sanitizers catch leaks easily there).
- [ ] `clang-tidy` clean on all new source files.
- [ ] T-count parity: `NativeRSProvider` within 1% of `GridSynthProvider`
      on Ross-Selinger 2016 Table 1 angles at ε = 10⁻¹⁰.
- [ ] T-count parity: `KliuchnikovProvider` within 5% of Qualtran
      `rotation_synthesis` on a 100-rotation corpus.
- [ ] Throughput: `NativeRSProvider` ≥ 5× faster than `GridSynthProvider`
      on a 10⁴ random angle corpus at matched ε.
- [ ] `SKProvider` renamed to `BFSTableProvider`; `SKProvider` retained
      as a deprecated alias.
- [ ] ADR-0008 transitions from "Accepted" to "Implemented".
- [ ] `memory-bank/activeContext.md` updated with Stage 7 decision.
- [ ] `memory-bank/progress.md` Stage 6 row marked complete.
- [ ] `CHANGELOG.md` updated.
- [ ] GitHub Milestone "Stage 6: Native Synthesis" closed.

---

## Source Layout (Stage 6 additions)

```
cmake/
  mpfr_config.cmake
  gmp_config.cmake

include/qfault/math/
  Real.hpp                              # MPFR wrapper
  Integer.hpp                           # GMP wrapper
  Z_omega.hpp                           # ring elements ℤ[ω]

src/qfault/math/
  Real.cpp
  Integer.cpp
  Z_omega.cpp

include/qfault/passes/synthesis/
  NativeRSProvider.hpp
  KliuchnikovProvider.hpp
  BFSTableProvider.hpp                  # renamed from SKProvider
  SKProvider.hpp                        # now a thin deprecated alias

src/qfault/passes/synthesis/
  NativeRSProvider.cpp
  KliuchnikovProvider.cpp
  BFSTableProvider.cpp

tests/unit/
  test_Real.cpp
  test_Integer.cpp
  test_Z_omega.cpp
  test_NativeRSProvider.cpp
  test_KliuchnikovProvider.cpp
  test_BFSTableProvider.cpp             # renamed

bench/tier1/
  native_rs_vs_gridsynth.sh
  kliuchnikov_vs_qualtran.sh
```

---

## Risks and Mitigations

| Risk | Mitigation |
|------|-----------|
| Ross-Selinger numerics tricky to get right (rounding, MPFR precision settings) | Start with a port of newsynth's algorithm; validate every step against newsynth output |
| Kliuchnikov-2023 mixed-fallback is more complex than Ross-Selinger | Treat as a separate epic with its own kickoff sub-document; budget 8 weeks for 6-C alone |
| MPFR + GMP add a portability burden | FetchContent + system find_package dual-path; document Windows/macOS quirks in `cmake/mpfr_config.cmake` |
| 5× throughput target may not be achievable on small ε | Document the curve: throughput at ε = 10⁻⁴ vs 10⁻¹⁰; the 5× target applies at the v0.1 ε default |
| `KliuchnikovProvider` RUS introduces measurement-conditional gates that conflict with the LOGICAL IR level | New ADR may be needed if RUS requires IR-level changes; flag at start of 6-C-3 |
| Stage 6 schedule pressure | Stage is **optional / contingent**; can slip to v0.2 without invalidating v0.1 |
