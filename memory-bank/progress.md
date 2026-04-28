# Progress Log — QFault

Append-only. Never delete. Newest entries at the top of each section.
> Single source of truth for "what is shipped". Update on every story completion.
---

## Stage Status Overview

| Stage | Status | Tests | Code | Docs |
|-------|--------|-------|------|------|
| 1 — IR + PassManager + QASM 3.0 | ✅ COMPLETE | 93/93 | shipped | exit-report ✅ |
| 2 — T-gate synthesis | ✅ CODE COMPLETE (gate pending) | 118/118 | shipped | exit-report pending GridSynth install |
| 2.5 — Verification & Reproducibility Harness | 🔜 NEXT | 0 | not started | spec ready |
| 3 — Lattice Surgery Mapper | 🚧 PLANNED | 0 | not started | spec + todo + kickoff + prompt_plan ready |
| 4 — MSD + Resource Estimator | 🚧 PLANNED | 0 | not started | kickoff stub |
| 5a — Output backends | 🚧 PLANNED | 0 | not started | kickoff stub |
| 5b — Python bindings | 🚧 PLANNED | 0 | not started | kickoff stub |
| 5c — Paper + Zenodo + v0.1.0 release | 🚧 PLANNED | n/a | not started | kickoff stub |
| 6 — Native Ross-Selinger + Kliuchnikov-2023 | 🚧 OPTIONAL | 0 | not started | kickoff stub |
| 7 — Formal methods OR MLIR | 🚧 OPTIONAL | 0 | not started | kickoff stub |

---

## v0.2 Audit Outcomes (2026-04-26)

A defensive portfolio audit was conducted to identify claims that would not
survive academic / hiring scrutiny. Key findings, with their resolution:

| Finding | Resolution |
|---------|-----------|
| "C++ throughput on synthesis" claim is false — `GridSynthProvider` is a `popen` wrapper around the Haskell binary | Disclosed in ADR-0013; closed by Stage 6 (native Ross-Selinger / Kliuchnikov) |
| `SKProvider` is a depth-7 BFS over a 512-entry table with ~1e-3 accuracy — calling it "Solovay-Kitaev" is a stretch | Renamed `SKProvider` → `BFSTableProvider` in v0.2; reframed as a fallback / sanity oracle (ADR-0013) |
| "Verification" against Stim is actually validation — Stim catches stabiliser-level errors but not logical equivalence | Reframed as validation in ADR-0009; Stage 2.5 adds MQT QCEC for logical equivalence |
| ADR-0006 routing claim ("greedy Manhattan") is too weak for a CGO27 Tools-track submission | Strengthened in ADR-0006 to A\* + Litinski templates + Silva 2024 EAF |
| ADR-0007 MSD factory choice was hand-waved | Strengthened in ADR-0007: catalog enumeration over Beverland 2022 Table VII assembled factories |
| No conference target ladder | ADR-0016 sets QCE26 → CGO27 → OOPSLA / PLDI gating |
| No reproducibility infrastructure | ADR-0017 mandates Dockerfile + flake.nix + Zenodo DOI + `papers/` directory |
| No formal-methods deliverable | ADR-0018 (Draft) and Stage 7 spec lay out MLIR / Coq / SMT options |
| Stage 6 (native synthesis) was not on the roadmap | New stage added; spec at `docs/phases/stage-6-native-synthesis/` |

---

## Stage 1: IR + Pass Manager Core

**Target:** 3–4 weeks | **Gate:** IR cleanly represents both logical AND physical ops
**Actual:** 1 day (AI-assisted)

### Progress Entries

- **2026-04-25** — Stage 1 complete. 93/93 tests green on gcc-13 and clang-18 with
  `-Werror`. ASAN+UBSAN clean (after fixing signed integer overflow in
  test_PassContext::TimerMeasuresElapsedTime — `volatile int` → `volatile long long`).
  ADR-0001 Accepted. All 18 GitHub issues populated with descriptions and closed.

- **2026-04-24** — Project initialised. Documentation skeleton complete.
  All memory-bank files, ADRs, phase docs, and Claude commands written.

### Stage 1 Decisions Log

| Date | Decision | Rationale |
|------|----------|-----------|
| 2026-04-25 | ADR-0001 Accepted | Integration test confirmed variant<LogicalGate,PatchOp> viable |
| 2026-04-24 | Two-level IR with discriminated union | Avoids two separate IR types and a lossy lowering pass |
| 2026-04-24 | Global code distance d for v0.1 | Variable-d routing dramatically complicates scheduling; deferred to v0.2 |

---

## Stage 2: Synthesis Pass (T-Gate)

**Target:** 3–4 weeks
**Gate:** T-count within 1% of GridSynth reference; overhead ≤5% on 1000-gate circuit

### Progress Entries

- **2026-04-26** — v0.2 audit complete. Disclosed in ADR-0013 that
  `GridSynthProvider` is a `popen` wrapper, not native C++ synthesis. Renamed
  `SKProvider` → `BFSTableProvider` planned for v0.2. Stage 6 added to the
  roadmap to close the native-synthesis gap.

- **2026-04-25** — Stage 2 implementation complete. All 10 GitHub issues (#19–#28)
  closed. 118/118 tests green on gcc-13, clang-18 (debug + release), and
  clang-18-asan. Stage gate benchmark pending GridSynth binary installation.

- **2026-04-25** — Stage 2 planning complete. Spec, todo, and kickoff documents
  written. ADRs 0002/0003/0004 confirmed Accepted from Stage 1.

---

## Stage 2.5: Verify + Bench — planned 2026-04-26 (NEW STAGE)

Acceptance: `make figures` reproduces published Stage 2 numbers within 5% on a
clean Ubuntu 24.04 container, with all external binaries pinned by version.

- [ ] Stim v1.15.0 FetchContent integration (libstim target, SIMD-width=64)
- [ ] MQT QCEC v3.5.0 integration (FetchContent the C++ source)
- [ ] `verify/` library target with Stim oracle + QCEC bridge
- [ ] BFSTableProvider rename (ADR-0013) + deprecated alias for SKProvider
- [ ] `bench/circuits/` git submodules: QASMBench, MQT Bench (generator), Feynman
- [ ] `bench/golden/` initial committed JSON tables
- [ ] `make figures` target → `bench/plots/*.pdf`
- [ ] Dockerfile (ubuntu:24.04, multi-stage, all deps version-pinned)
- [ ] `flake.nix` for Nix-preferring reviewers
- [ ] Zenodo DOI integration on tagged release
- [ ] `papers/` directory skeleton

---

## Stage 3: Lattice Surgery Mapper — planned (full treatment in `docs/phases/stage-3-lattice-surgery/`)

Acceptance: BV-10 logical circuit at d=5 produces correct logical output
verified by `stim::Circuit::has_flow(...)` for all 10 logical Z observables;
tile-count formulas reproduce within 10% (compact 1.5n+3, intermediate 2n+4,
fast 2n+√(8n)+1).

See `docs/phases/stage-3-lattice-surgery/spec.md` for the full breakdown.

---

## Stage 4: MSD Scheduling + Resource Estimator

*(not started)*

---

## Stage 5: Output Backends + Python + arXiv

*(not started)*

---

## Stage 6: Native Synthesis (post-arXiv, optional)

**Target:** 6–10 weeks post-Stage 5
**Gate:** Native synthesis T-counts within 1 % of Haskell GridSynth on a 100-circuit
corpus; end-to-end runtime ≤ 2× the subprocess baseline.

*(not started — post-Stage 5 deliverable per ADR-0013)*

---

## Stage 7: Formal-Methods or MLIR Stretch

**Target:** open-ended (≥ 3 months collaborator commitment required)
**Gate:** per-option, see `docs/phases/stage-7-formal-stretch/spec.md`

*(opt-in only — gates OOPSLA / PLDI submission per ADR-0016)*

---

## Benchmark Baseline (to be established at Stage 5 completion)

| Benchmark | Circuit | QFault | GridSynth standalone | PyLIQTR |
|-----------|---------|--------|---------------------|---------|
| T-count | 100-gate random Clifford+T | — | — | — |
| Physical qubits | Bernstein-Vazirani n=10 | — | — | — |
| Compile time | BV n=10 | — | — | — |

---

## arXiv Preprint Status

- [ ] Stage 5 complete
- [ ] Benchmark tables populated
- [ ] Honest limitations section drafted (per v0.2 audit findings)
- [ ] Abstract drafted
- [ ] Introduction written
- [ ] Related work section written
- [ ] Submitted to arXiv
- [ ] Zenodo DOI minted (per ADR-0017)

---

## Conference Submission Status (per ADR-0016)

| Conference | Status | Submission target |
|-----------|--------|-------------------|
| IEEE QCE 2026 | ⬜ pending Stage 5 | ~Month 7 |
| CGO 2027 (Tools track) | ⬜ pending Stage 5 + 2.5 | ~Month 12 |
| TQC 2026 | ⬜ optional alternative to QCE | ~Month 7 |
| OOPSLA / PLDI | ⬜ gated on Stage 7 | open-ended |

---

## Community Signals

*(record any issues, emails, forum posts from the QEC community)*

| Date | Signal | Source |
|------|--------|--------|
| — | *(none yet)* | — |
