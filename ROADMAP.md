# QFault Roadmap

> **Note (v0.2 audit, 2026-04-26):** This roadmap was extended from the
> original 5-stage plan to a 7-stage plan to reflect findings from the
> v0.2 portfolio defense audit. New stages: 2.5 (Verify+Bench), 6
> (Native Synthesis), 7 (Formal/MLIR Stretch — optional). See
> ADR-0008 / ADR-0009 / ADR-0013 / ADR-0016 / ADR-0018 for the rationale.

## Timeline Overview

```
Month 1   Month 2   Month 3   Month 3.5  Month 4   Month 5-6  Month 6-7   Month 7-9   Month 9+
│         │         │         │          │         │          │           │           │
▼         ▼         ▼         ▼          ▼         ▼          ▼           ▼           ▼
┌──────┐ ┌──────┐ ┌──────┐ ┌────────┐ ┌──────┐ ┌──────┐ ┌──────────┐ ┌──────────┐ ┌──────────┐
│Stage1│ │Stage2│ │Stage3│ │Stage2.5│ │Stage4│ │Stage5│ │  arXiv   │ │ Stage 6  │ │ Stage 7  │
│  IR  │ │Synth │ │Lattice│ │Verify │ │ MSD  │ │Output│ │ Preprint │ │ Native   │ │ Formal/  │
│PassMg│ │Pass  │ │Surgery│ │+ Bench │ │ResEst│ │Pyth  │ │ + v0.1.0 │ │ Synth    │ │ MLIR     │
└──────┘ └──────┘ └──────┘ └────────┘ └──────┘ └──────┘ └──────────┘ │ (opt)    │ │ (opt)    │
                                                                     └──────────┘ └──────────┘
✅ DONE   ✅ DONE   active    next      next      next                              QCE26       OOPSLA gate
                                                                                   CGO27        per ADR-0016
```

---

## Stage 1: IR + Pass Manager Core (Weeks 1–4) ✅ COMPLETE

**Stage Gate:** IR cleanly represents both logical Clifford+T AND surface code
patch ops in one data structure without lossy conversion. NoOpPass round-trip
test passes. **Result: 93/93 tests green on gcc-13 + clang-18 + ASAN/UBSAN.**

---

## Stage 2: Synthesis Pass — T-Gate Decomposition (Weeks 4–9) ✅ CODE COMPLETE

**Stage Gate:** `SynthesisProvider` abstraction adds ≤ 5% runtime overhead
vs calling GridSynth directly on a 1,000-gate circuit. **Code complete:
118/118 tests green; formal gate pending GridSynth binary install.**

**Important caveat (audit finding):** `GridSynthProvider` is a `popen`
wrapper around the Haskell binary — it is _not_ native C++ synthesis.
Removing this caveat is the goal of Stage 6. (See ADR-0008, ADR-0013.)

---

## Stage 3: Lattice Surgery Mapper (Weeks 9–16)

**Stage Gate:** For a 10-logical-qubit Bernstein-Vazirani circuit, A* lattice
surgery routing with Litinski 2019 templates and Silva 2024 EAF scheduling
produces a patch schedule that **Stim v1.15.0** simulates with correct logical
output at d=5. (See ADR-0006.)

| Deliverable | Description |
|-------------|-------------|
| `LatticeSurgeryPass` | Logical CNOT → MERGE/SPLIT/MEASURE sequences |
| Patch grid model | 2D `PatchCoord` grid; bus-patch routing |
| **A\* router** | Sanitizer-clean implementation; not the liblsqecc Dijkstra default |
| **Litinski layout templates** | compact 1.5n+3 / 9τ; intermediate 2n+4 / 5τ; fast 2n+√(8n)+1 / 1τ |
| **EAF scheduler** | Silva 2024 EAF over the patch grid |
| **Stim v1.15.0 oracle** | Native C++ FetchContent with `libstim` target |
| BV-10 d=5 reference | 600–800 physical qubits expected for 11 logical × (2d²−1) |

**GitHub Milestone:** `Stage 3: Lattice Surgery Mapper`

---

## Stage 2.5: Verify + Bench (insert between Stage 3 and Stage 4)

**New stage from v0.2 audit.** ADR-0009 frames this as **validation**, not
verification — Stim catches stabiliser-level errors; QCEC catches logical
equivalence failures. Neither is a correctness proof (that is Stage 7).

**Stage Gate:** Stim oracle + MQT QCEC v3.5.0 equivalence checker run as CI
gates on every push. The 50-circuit regression corpus passes both.

| Deliverable | Description |
|-------------|-------------|
| `cmake/stim_config.cmake` | Stim v1.15.0 via FetchContent, `libstim` target, `SIMD_WIDTH=64` |
| `cmake/qcec_config.cmake` | MQT QCEC v3.5.0 with `BUILD_MQT_QCEC_BINDINGS=OFF` |
| `scripts/compare-stim.sh` | Replaces the Stage 1 stub |
| `scripts/qcec-equivalence.sh` | New: QCEC equivalence check pre/post compilation |
| 50-circuit corpus | BV, QFT, adder, small Shor pieces; committed under `tests/reference/` |

**GitHub Milestone:** `Stage 2.5: Verify + Bench`

---

## Stage 4: MSD Scheduling + Resource Estimator (Weeks 16–23)

**Stage Gate:** MSD factory scheduler produces a lower physical qubit count
than the naïve "one factory per T-gate" baseline on a circuit with ≥50 T-gates.

| Deliverable | Description |
|-------------|-------------|
| MSD factory catalog | Beverland 2022 Table VII assembled factories (15-to-1 SE×1 d=9; 15-to-1 SE×16 d=5+RM-prep d=13; 15-to-1 SE×16 d=3+SE d=11) |
| Bravyi-Haah 2012 | 116-to-12 (alternative for high-throughput regimes) |
| Litinski 225-to-1 | optional alternative for high-fidelity regimes |
| `MSDSchedulerPass` | Earliest-available factory assignment with routing delays (per ADR-0007) |
| `ResourceEstimatorPass` | Compiler-derived counts (not heuristic), per ADR-0003 global d |
| `ResourceReport` | JSON + Markdown + LaTeX output |

**GitHub Milestone:** `Stage 4: MSD Scheduling + Resource Estimator`

---

## Stage 5: Output Backends + Python Bindings + arXiv (Weeks 23–28)

**Stage Gate:** arXiv preprint receives ≥1 substantive technical response
from the QEC research community within 4 weeks of posting.

| Deliverable | Description |
|-------------|-------------|
| QASM 3.0 backend | Standard QASM 3.0 + QFault stdlib (per ADR-0010) |
| QIR backend | QIR Alliance v0.1 base profile (per ADR-0005) |
| Stim backend | Native `.stim` emitter for the verification loop |
| **pybind11 bindings** | v2.11.1 pinned (per ADR-0012) |
| PyPI package | `pip install qfault` |
| arXiv preprint | Architecture, benchmarks, related work, **honest limitations** |
| v0.1.0 GitHub release | Tagged release with changelog |

**GitHub Milestone:** `Stage 5: Output + Release + arXiv`

---

## Stage 6: Native Synthesis (post-arXiv, optional, Months 7–9)

**New stage from v0.2 audit.** Closes the "GridSynth is a popen wrapper"
caveat that ADR-0013 documents as a v0.1 limitation.

**Stage Gate:** Native C++ synthesis provider produces T-counts within 1 %
of the Haskell GridSynth oracle on a 100-circuit corpus, with end-to-end
runtime ≤ 2× the GridSynth subprocess baseline.

| Deliverable | Description |
|-------------|-------------|
| `RossSelingerNativeProvider` | C++ implementation of Ross-Selinger 2016 |
| `KliuchnikovNativeProvider` | C++ implementation of Kliuchnikov 2023 (T-count parity) |
| Rename: `SKProvider` → `BFSTableProvider` | Honesty fix per ADR-0013 |
| Oracle harness | `tests/integration/test_native_vs_gridsynth.cpp` |

**GitHub Milestone:** `Stage 6: Native Synthesis` (optional)

---

## Stage 7: Formal-Methods or MLIR Stretch (open-ended, optional)

**Per ADR-0016, gates OOPSLA / PLDI submission.** Without Stage 7, the
project is restricted to QCE26 and CGO27 Tools track.

Pick **one** of three mutually-exclusive options:

- **Option A** — `qfault.fto` MLIR dialect (per ADR-0018). Round-trip
  through `qfault-opt` on a 50-circuit corpus.
- **Option B** — Coq / Rocq formalisation of one pass (lattice surgery
  primitives recommended).
- **Option C** — SMT-based per-compilation translation validator
  (`qfault-validate` CLI; ≥ 80 % PASS rate on regression corpus).

**GitHub Milestone:** `Stage 7: Stretch` (optional, opt-in only)

---

## Conference Target Ladder (per ADR-0016)

| Conference | Track | Gating stage | Realistic? |
|-----------|-------|--------------|------------|
| **IEEE QCE 2026** | full paper | Stage 5 (v0.1) | ✅ primary target |
| **CGO 2027** | Tools track | Stage 5 + Stage 2.5 | ✅ realistic |
| OOPSLA / PLDI | main track | Stage 7 (any option) | only with Stage 7 |
| TQC | full paper | Stage 5 + Stage 2.5 | optional alternative to QCE |

---

## Post-v0.1 Roadmap (v0.2+)

| Item | Priority | Rationale |
|------|----------|-----------|
| Variable code distance per circuit region | High | Lifts ADR-0003 v0.1 limitation |
| Native synthesis (Stage 6) | High | Closes ADR-0013 caveat |
| Phase-polynomial / ZX optimisation pass | Medium | Per ADR-0011 (currently Draft) |
| Colour code support | Medium | Broadens applicability beyond surface code |
| Backtracking router for >10k logical qubits | Low | Greedy A\* is sufficient for v0.1 |
| MLIR / formal stretch (Stage 7) | Stretch | Per ADR-0016 / ADR-0018 |

---

## Key Risk Mitigations

| Risk | Mitigation |
|------|-----------|
| Lattice surgery routing NP-hard | A\* heuristic with documented bounds; backtracker deferred |
| "GridSynth is a popen wrapper" perception | Explicitly disclosed in ADR-0013 / Stage 5 arXiv; Stage 6 closes it |
| QIR spec instability | Pin to QIR Alliance v0.1 base profile (ADR-0005); modular backend |
| No real hardware | Stim v1.15.0 + MQT QCEC v3.5.0 as twin oracles (ADR-0009) |
| pybind11 maintenance debt | Keep binding surface minimal; revisit nanobind in v0.2 (ADR-0012) |
| Context rot over 6+ months | memory-bank/ + CHANGELOG failed approaches log (ADR-0014) |
| Conference rejection cycles | Ladder targets QCE26 first, CGO27 next; OOPSLA gated on Stage 7 (ADR-0016) |
| liblsqecc license incompatibility | Do not vendor (GPL-3.0 vs Apache 2.0); reimplement A\* (ADR-0015) |
