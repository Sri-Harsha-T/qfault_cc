# Progress Log — QFault

Append-only. Never delete. Newest entries at the top of each section.

---

## Stage Status Overview

| Stage | Name | Status | Started | Completed | Gate Passed |
|-------|------|--------|---------|-----------|-------------|
| 1 | IR + Pass Manager Core | 🚧 In Progress | 2026-04-24 | — | ⬜ |
| 2 | Synthesis Pass (T-Gate) | ⬜ Backlog | — | — | ⬜ |
| 3 | Lattice Surgery Mapper | ⬜ Backlog | — | — | ⬜ |
| 4 | MSD Scheduling + Resource Estimator | ⬜ Backlog | — | — | ⬜ |
| 5 | Output Backends + Python + arXiv | ⬜ Backlog | — | — | ⬜ |

---

## Stage 1: IR + Pass Manager Core

**Target:** 3–4 weeks | **Gate:** IR cleanly represents both logical AND physical ops

### Progress Entries

- **2026-04-24** — Project initialised. Documentation skeleton complete.
  All memory-bank files, ADRs, phase docs, and Claude commands written.
  CMake and source code: not yet started.

### Stage 1 Known Issues

*(none yet)*

### Stage 1 Decisions Log

| Date | Decision | Rationale |
|------|----------|-----------|
| 2026-04-24 | Two-level IR with discriminated union | Avoids two separate IR types and a lossy lowering pass (tentative — revisit at gate) |
| 2026-04-24 | Global code distance d for v0.1 | Variable-d routing dramatically complicates scheduling; deferred to v0.2 |

---

## Stage 2: Synthesis Pass (T-Gate)

*(not started)*

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
