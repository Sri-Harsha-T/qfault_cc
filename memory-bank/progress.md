# Progress Log — QFault

Append-only. Never delete. Newest entries at the top of each section.

---

## Stage Status Overview

| Stage | Name | Status | Started | Completed | Gate Passed |
|-------|------|--------|---------|-----------|-------------|
| 1 | IR + Pass Manager Core | ✅ Complete | 2026-04-24 | 2026-04-25 | ✅ 2026-04-25 |
| 2 | Synthesis Pass (T-Gate) | 🔄 Code Complete | 2026-04-25 | — | ⬜ Gate pending GridSynth |
| 3 | Lattice Surgery Mapper | ⬜ Backlog | — | — | ⬜ |
| 2.5 | Verify + Bench (Stim + QCEC) | ⬜ Backlog (post-Stage 3) | — | — | ⬜ |
| 4 | MSD Scheduling + Resource Estimator | ⬜ Backlog | — | — | ⬜ |
| 5 | Output Backends + Python + arXiv | ⬜ Backlog | — | — | ⬜ |
| 6 | Native Synthesis (post-arXiv) | ⬜ Optional, post-Stage 5 | — | — | ⬜ |
| 7 | Formal-Methods or MLIR Stretch | ⬜ Optional, opt-in only | — | — | ⬜ |

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

## Stage 2.5: Verify + Bench

**Target:** 1–2 weeks (parallel with end of Stage 3)
**Gate:** Stim oracle + MQT QCEC equivalence checker run as CI gates;
50-circuit regression corpus passes both.

*(not started — post-Stage 3 deliverable per ADR-0009)*

---

## Stage 3: Lattice Surgery Mapper

*(active — see `docs/phases/stage-3-lattice/spec.md`)*

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
