# Stage 6 Kickoff — Native Synthesis

**Stage:** 6 of 7 (optional / contingent)
**Milestone:** "Stage 6: Native Synthesis"
**Prerequisite:** Stage 5 (release) closed; v0.1 published

---

## Why this stage exists

The v0.1 audit (per ADR-0008) identified two gaps versus the
synthesis-portfolio frontier:

1. **`GridSynthProvider` is a `popen` wrapper** around the Haskell
   `gridsynth` binary, so the "C++ throughput on synthesis" pitch does
   not apply to v0.1. ADR-0004 retracted that claim explicitly.

2. **Qualtran ships both Ross-Selinger AND Kliuchnikov-2023** mixed
   fallback / repeat-until-success. QFault v0.1 ships only one
   algorithm.

Stage 6 closes both gaps as native C++ providers behind the existing
`SynthesisProvider` Concept (ADR-0002). The Concept boundary was designed
exactly so this addition would be zero-friction at the call site.

This stage is **optional / contingent** on collaborator availability and
on Stage 5c (the QCE26 submission) finishing on schedule. If Stage 5c
runs long, Stage 6 slips to v0.2 without invalidating Stage 5's release.

---

## Objectives

1. **`NativeRSProvider`** — native C++ Ross-Selinger using MPFR (4.2) +
   GMP (6.3) for exact arithmetic over ℤ[ω].
2. **`KliuchnikovProvider`** — Kliuchnikov-2023 mixed-fallback / RUS
   protocol as a second native provider.
3. **Rename `SKProvider` → `BFSTableProvider`** per ADR-0013 (preserve
   `SKProvider` as a deprecated alias for one release).
4. **Concept-boundary regression**: extend `test_SynthesisProvider.cpp`
   with positive `static_assert`s for the two new providers; the
   parameterized integration test exercises all four.

**Non-goals for Stage 6:**
- No new IR-level features (the RUS protocol may introduce
  measurement-conditional gates; if so, raise an ADR).
- No new passes other than the providers themselves.
- No new backends.
- No Python-binding changes.

---

## Key Design Decisions Going In

1. **MPFR + GMP** are the standard tools for the exact arithmetic needed
   by Ross-Selinger. Pin MPFR ≥ 4.2 and GMP ≥ 6.3 in
   `cmake/dependency_versions.cmake`. The Dockerfile builder stage
   already includes them per ADR-0017.

2. **`qfault::math::Real` and `qfault::math::Integer`** wrap raw `mpfr_t`
   / `mpz_t` so the rest of the codebase never sees MPFR/GMP types
   directly. This isolates ABI risk if MPFR/GMP versions change.

3. **`qfault::math::Z_omega`** for elements of ℤ[ω] (ω = e^{iπ/4}); this
   is the central data type for both Ross-Selinger and Kliuchnikov.

4. **The TGateSynthesisPass call site does NOT change.** The new
   providers slot in by template instantiation:
   `pm.add<TGateSynthesisPass<NativeRSProvider>>(NativeRSProvider{})`.

---

## Execution Order

```
6-A-1/2/3/4 MPFR + GMP wrappers
    │
    ▼
6-D-1/2/3/4 SKProvider → BFSTableProvider rename (small; do early)
    │
    ▼
6-B-1/2/3/4/5/6 NativeRSProvider (the throughput target)
    │
    ▼
6-C-1/2/3/4 KliuchnikovProvider (the algorithm-portfolio target)
    │
    ▼
6-E-1/2/3 Concept-boundary regression
    │
    ▼
Stage Gate
```

Epic 6-D (rename) is small and can land first to clean up the symbol
namespace before introducing two new providers. Epics 6-A and 6-B can
overlap once 6-A-3 (the wrapper namespace) lands.

---

## Tools and Dependencies

| Tool | Version | Source |
|------|---------|--------|
| MPFR | ≥ 4.2 | FetchContent or system find_package |
| GMP | ≥ 6.3 | FetchContent or system find_package |
| Qualtran (reference) | latest | for T-count parity comparison only — do NOT vendor |
| newsynth (reference) | latest commit | for Ross-Selinger algorithm validation — do NOT vendor |

---

## Success Criteria (from spec.md)

> The native C++ Ross-Selinger provider achieves ≥ 5× throughput over the
> `gridsynth` subprocess wrapper at matched T-count and ε = 10⁻¹⁰, on a
> corpus of 10⁴ random rotations. T-count parity with the Haskell
> reference is within 1% on the Ross-Selinger 2016 Table 1 angles. The
> Kliuchnikov provider matches Qualtran's published T-counts within 5%
> on a 100-rotation regression corpus.

This is verifiable by `bench/tier1/native_rs_vs_gridsynth.sh` and
`bench/tier1/kliuchnikov_vs_qualtran.sh`. The CI job runs both at the
end of Stage 6.

---

## Risks to monitor at kickoff

- **Numerics first**: Ross-Selinger is a numerics-heavy algorithm. A
  port of newsynth's algorithm step-by-step, validated against newsynth
  output at every stage, is the safest path. Do NOT rewrite from the
  paper alone.
- **MPFR precision settings** affect both correctness and throughput.
  Document the precision-vs-speed curve in
  `bench/golden/native_rs_precision_curve.csv`.
- **Kliuchnikov RUS may require IR-level changes** if the RUS measurement
  feedback cannot be expressed as a flat sequence of LogicalGates. If
  this hits during 6-C-3, raise an ADR before continuing.
