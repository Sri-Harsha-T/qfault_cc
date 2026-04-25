# Progress Log — QFault

Append-only. Never delete. Newest entries at the top of each section.

---

## Stage Status Overview

| Stage | Name | Status | Started | Completed | Gate Passed |
|-------|------|--------|---------|-----------|-------------|
| 1 | IR + Pass Manager Core | ✅ Complete | 2026-04-24 | 2026-04-25 | ✅ 2026-04-25 |
| 2 | Synthesis Pass (T-Gate) | 🔄 Code Complete | 2026-04-25 | — | ⬜ Gate pending GridSynth |
| 3 | Lattice Surgery Mapper | ⬜ Backlog | — | — | ⬜ |
| 4 | MSD Scheduling + Resource Estimator | ⬜ Backlog | — | — | ⬜ |
| 5 | Output Backends + Python + arXiv | ⬜ Backlog | — | — | ⬜ |

---

## Stage 1: IR + Pass Manager Core

**Target:** 3–4 weeks | **Gate:** IR cleanly represents both logical AND physical ops
**Actual:** 1 day (AI-assisted)

### Progress Entries

- **2026-04-25** — Stage 1 complete. 93/93 tests green on gcc-13 and clang-18 with
  `-Werror`. ASAN+UBSAN clean (after fixing signed integer overflow in
  test_PassContext::TimerMeasuresElapsedTime — `volatile int` → `volatile long long`).
  ADR-0001 Accepted. All 18 GitHub issues populated with descriptions and closed.
  Stage 1 phase docs (exit-report, kickoff, todo, spec) fully updated.
  Stage 2 spec, todo, and kickoff documents written.

- **2026-04-24** — Project initialised. Documentation skeleton complete.
  All memory-bank files, ADRs, phase docs, and Claude commands written.
  CMake and source code: not yet started.

### Stage 1 Known Issues

- UBSAN signed integer overflow in `test_PassContext::TimerMeasuresElapsedTime`
  (sum of 0..99999 overflows `int`). Fixed: `volatile int` → `volatile long long`.
  Commit: (UBSAN fix commit — see git log)

### Stage 1 Decisions Log

| Date | Decision | Rationale |
|------|----------|-----------|
| 2026-04-25 | ADR-0001 Accepted | Integration test `test_ir_two_level.cpp` confirmed variant<LogicalGate,PatchOp> viable; no structural incompatibility found |
| 2026-04-24 | Two-level IR with discriminated union | Avoids two separate IR types and a lossy lowering pass |
| 2026-04-24 | Global code distance d for v0.1 | Variable-d routing dramatically complicates scheduling; deferred to v0.2 |

---

## Stage 2: Synthesis Pass (T-Gate)

**Target:** 3–4 weeks
**Gate:** T-count within 1% of GridSynth reference; overhead ≤5% on 1000-gate circuit

### Progress Entries

- **2026-04-25** — Stage 2 implementation complete. All 10 GitHub issues (#19–#28)
  implemented and closed. 118/118 tests green on gcc-13, clang-18 (debug + release),
  and clang-18-asan (ASAN+UBSAN clean). Deliverables: SynthesisProvider Concept,
  GridSynthProvider (subprocess), SKProvider (pure C++, depth-7 BFS), TGateSynthesisPass,
  integration tests, T-count validation test (GTEST_SKIP without binary),
  bench-synthesis.sh. Stage gate benchmark pending GridSynth binary installation.

- **2026-04-25** — Stage 2 planning complete. Spec, todo, and kickoff documents written
  at `docs/phases/stage-2-synthesis/`. ADRs 0002/0003/0004 confirmed Accepted from Stage 1.

### Stage 2 Decisions Log

*(none yet — ADRs pending)*

---

## Stage 3: Lattice Surgery Mapper

*(not started)*

---

## Stage 4: MSD Scheduling + Resource Estimator

*(not started)*

---

## Stage 5: Output Backends + Python + arXiv

*(not started)*

---

## Benchmark Baseline (to be established at Stage 2 completion)

| Benchmark | Circuit | QFault | GridSynth standalone | PyLIQTR |
|-----------|---------|--------|---------------------|---------|
| T-count | 100-gate random Clifford+T | — | — | — |
| Physical qubits | Bernstein-Vazirani n=10 | — | — | — |
| Compile time | BV n=10 | — | — | — |

---

## arXiv Preprint Status

- [ ] Stage 5 complete
- [ ] Benchmark tables populated
- [ ] Abstract drafted
- [ ] Introduction written
- [ ] Related work section written
- [ ] Submitted to arXiv

---

## Community Signals

*(record any issues, emails, forum posts from the QEC community)*

| Date | Signal | Source |
|------|--------|--------|
| — | *(none yet)* | — |
