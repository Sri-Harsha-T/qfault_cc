# Stage 2.5 Kickoff — Verify + Bench

**Stage:** 2.5 of 7 (inserted)
**Milestone:** "Stage 2.5: Verify + Bench"
**Prerequisite:** Stage 2 closed

---

## Why this stage exists

Stage 3's "Stim says correct" gate (per `docs/phases/stage-3-lattice-surgery/spec.md`)
requires Stim linked, MQT QCEC bridged, golden circuits committed, and a
`make figures` reproducibility command working. Stage 2's exit report did
not include this plumbing because it was always meant to land between
Stages 2 and 3 — but the original 5-stage roadmap omitted it. The audit
of v0.1 surfaced this gap and Stage 2.5 fills it.

Per **ADR-0009** (verification strategy), the validation layer comprises
Stim + QCEC + golden-circuit regression. Per **ADR-0017** (reproducibility
infrastructure), the artifact bundle requires Dockerfile + flake.nix +
papers/ scaffolding.

---

## Objectives

1. **Stim integration** as a C++ library (FetchContent, like GoogleTest),
   pinned to v1.15.0.
2. **MQT QCEC bridge** as a CMake-buildable C++ component, pinned to
   v3.5.0.
3. **Benchmark corpus** as committed git submodules + a
   `bench/_generated/` Python-driven workflow.
4. **Tier 1 / Tier 2 benchmark harnesses** with deterministic CSV +
   plot output.
5. **Dockerfile + flake.nix** as the reproducibility entry points.

**Non-goals for Stage 2.5:**
- No lattice surgery (Stage 3).
- No new synthesis algorithms (Stage 6).
- No Python bindings (Stage 5b).
- No QIR / QASM 3.0 emitter (Stage 5a).

---

## Key Design Decisions Going In

1. **Stim's C++ ABI is unstable** — pin to v1.15.0 by tag, never main.
   Library target is `libstim` (already includes the `lib` prefix).
   `SIMD_WIDTH=64` for golden-test reproducibility.
2. **MQT QCEC** disable Construction checker (`Construction OFF`),
   alternating + simulation + ZX ON.
3. **Benchmark dispatch:** Tier 1 = single-rotation; Tier 2 =
   algorithm-level Clifford+T; Tier 3 = full FT pipeline (Stage 3+
   gates Tier 3).
4. **`cmake/dependency_versions.cmake`** is the single source of truth
   read by both `CMakeLists.txt` and `Dockerfile` (via build args) and
   referenced from `flake.nix`.

---

## Execution Order

```
2.5-D-3 dependency_versions.cmake
    │
    ▼
2.5-A-1 Stim FetchContent ──► 2.5-A-2 Stim oracle helper ──► 2.5-A-3 detector backstop ──► 2.5-A-4 SIMD-width discipline
2.5-B-1 QCEC FetchContent ──► 2.5-B-2 QCEC bridge       ──► 2.5-B-3 qubit-threshold ──► 2.5-B-4 QCEC golden circuits
2.5-C-1/2/3 corpus submodules + generators
                                     │
                                     ▼
                            2.5-C-4 Tier 1 harness
                            2.5-C-5 Tier 2 harness
                                     │
                                     ▼
                            2.5-D-1 Dockerfile
                            2.5-D-2 flake.nix
                            2.5-D-4 make figures
                                     │
                                     ▼
                                Stage Gate
```

Epics 2.5-A, 2.5-B, 2.5-C can run in parallel after 2.5-D-3 lands.
Epic 2.5-D depends on completion of A/B/C.

---

## Tools and Dependencies

| Tool | Version | Source |
|------|---------|--------|
| Stim | v1.15.0 | FetchContent |
| MQT QCEC | v3.5.0 | FetchContent |
| QASMBench | shallow submodule | git submodule |
| Feynman | shallow submodule | git submodule |
| MQT Bench | latest pip | `pip install mqt.bench` in Dockerfile builder stage |
| Docker | latest stable | host |
| Nix | latest with flakes | host (optional) |
| matplotlib | pinned via `bench/scripts/requirements.txt` | pip |

---

## Success Criteria (from spec.md)

> Running `make figures` on a clean `ubuntu:24.04` Docker container
> reproduces the published Stage 2 numbers within 5%, with all external
> binaries pinned by version. `tests/integration/test_stim_oracle.cpp`
> and `tests/integration/test_qcec_bridge.cpp` both pass against
> committed golden circuits.

This is verifiable: a CI job runs `docker build . && docker run ...
make figures` and diffs against `bench/golden/stage2_baseline.csv`.
