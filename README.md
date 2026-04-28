# QFault — Fault-Tolerant Quantum Compiler (C++20)

[![CI](https://github.com/Sri-Harsha-T/qfault_cc/actions/workflows/ci.yml/badge.svg)](https://github.com/Sri-Harsha-T/qfault_cc/actions/workflows/ci.yml)
[![License: Apache 2.0](https://img.shields.io/badge/License-Apache_2.0-blue.svg)](LICENSE)
[![C++20](https://img.shields.io/badge/C%2B%2B-20-blue.svg)](https://en.cppreference.com/w/cpp/20)

QFault is an open-source, modular C++20 compiler-pass library that takes a
logical quantum circuit (Clifford+T) and compiles it to fault-tolerant gate
sequences for surface-code execution, validated against the Stim simulator
and the MQT QCEC equivalence checker.

> **Status (April 2026):** Stages 1 and 2 are complete (211/211 passing tests).
> Stage 3 (lattice surgery routing) is the active work. v0.1.0 release ETA Q4 2026.
> Numerical claims, scope, and design decisions are recorded as
> [Architecture Decision Records](docs/adr/README.md).

---

## What QFault is

A unified, surface-code-first **C++20 compiler-infrastructure project** for
fault-tolerant quantum computing. Researchers and quantum systems engineers
currently glue 4–5 disconnected tools together (Stim, GridSynth, PyLIQTR,
newsynth, custom routing scripts) to compile a logical algorithm to
fault-tolerant instructions. QFault provides a single C++ codebase that
traverses logical-circuit IR → T-gate synthesis → lattice surgery routing →
magic-state-distillation scheduling → resource estimation → multi-backend
emission, with an LLVM-inspired `PassManager` and a C++20 Concept-based
`SynthesisProvider` interface.

## What QFault is *not* (read this before claiming benchmarks)

- **Not a faster GridSynth.** v0.1's `GridSynthProvider` is a `popen` subprocess
  wrapper around the Haskell `gridsynth` binary. C++ throughput claims apply
  to the surrounding pipeline (PassManager dispatch, IR traversal, lattice
  surgery routing, MSD scheduling), **not** to single-rotation synthesis.
  See [ADR-0004](docs/adr/0004-gridsynth-as-default.md).
- **Not a real Solovay-Kitaev implementation.** `SKProvider` (renamed to
  `BFSTableProvider` in v0.2 — see [ADR-0013](docs/adr/0013-bfs-table-provider-rename.md))
  is a depth-7 BFS over a 512-entry generator table with achievable accuracy
  bounded around `1e-3` Frobenius distance. It is a sanity oracle for testing
  without GridSynth, **not** a production synthesiser.
- **Not yet a Kliuchnikov-2023 contender.** Mixed-fallback / repeat-until-success
  synthesis is a Stage 6 deliverable, not a v0.1 feature.
- **Not formally verified.** QFault ships **validation, not verification**:
  Stim tableau-equivalence on Clifford segments + MQT QCEC equivalence on
  Clifford+T blocks ≤ 8 qubits as CI gates. Coq pass proofs are an optional
  Stage 7 stretch ([ADR-0009](docs/adr/0009-validation-not-verification.md)).

## What's actually shipped

| Stage | Component | Status | Test count |
|-------|-----------|--------|-----------|
| 1 | IR + PassManager + QASM 3.0 frontend | ✅ Complete | 93 |
| 2 | T-gate synthesis (`GridSynthProvider`, `BFSTableProvider`) | ✅ Code complete | 118 |
| 2.5 | Stim + MQT QCEC integration, Dockerfile, flake.nix | 🔜 Next | — |
| 3 | Lattice surgery routing (A* + Litinski templates) | 🚧 Planned | — |
| 4 | MSD scheduling + Beverland 2022 factory catalog | 🚧 Planned | — |
| 5 | Output backends (QASM 3.0, QIR, Stim native) + Python | 🚧 Planned | — |
| 6 | Native Ross-Selinger + Kliuchnikov-2023 (optional) | 🚧 Planned | — |
| 7 | Formal-methods or MLIR stretch (optional) | 🚧 Planned | — |

The full roadmap with falsifiable stage gates is in [ROADMAP.md](ROADMAP.md).

## Quick start

```bash
# Build (requires CMake ≥ 3.21, gcc-13 or clang-18)
cmake --preset gcc13-debug
cmake --build build/gcc13-debug -j
ctest --test-dir build/gcc13-debug --output-on-failure

# Optional: install GridSynth for production synthesis
#   https://github.com/kenmcken/newsynth (Haskell-based)
# Without GridSynth, Stage 2 tests use BFSTableProvider (sanity oracle only)

# Run the quick-test loop (<60s)
./scripts/quick-test.sh
```

## Project structure

- **`include/qfault/`** — public C++ API (header-only IR types, PassManager,
  Concept definitions). Path-scoped rules live in `.claude/rules/cpp.md` and
  `.claude/rules/qec.md`.
- **`src/qfault/`** — pass implementations and frontend
- **`tests/{unit,integration,reference}/`** — GoogleTest-based test suite
- **`docs/adr/`** — Architecture Decision Records (MADR-lite, 18 ADRs)
- **`docs/phases/stage-N-*/`** — per-stage spec, todo, kickoff, exit-report,
  prompt_plan
- **`memory-bank/`** — Cline-style persistent context for AI sessions
- **`bench/`** — Tier 1/2/3 benchmark harness, golden files, plots
- **`verify/`** — Stim oracle and MQT QCEC equivalence checks (Stage 2.5+)
- **`.claude/`** — CCPM project-management workflow (slash commands, agents,
  path-scoped rules, ccpm skill, settings)

## Reproducibility (artifact-evaluation target)

QFault targets the **ACM triple-badge** (Available + Reusable + Reproduced)
for the planned QCE26/CGO27 submissions ([ADR-0017](docs/adr/0017-reproducibility-infrastructure.md)).
The reproducibility apparatus (Stage 2.5 deliverables) includes:

- **Pinned external dependencies**: Stim v1.15.0, MQT QCEC v3.5.0,
  GoogleTest v1.14, pybind11 v2.11.1, GridSynth (Haskell binary, version-pinned)
- **Dockerfile** based on `ubuntu:24.04` with multi-stage builds
- **`flake.nix`** for Nix-preferring reviewers and bit-exact rebuilds
- **`make figures`** target reproducing all paper plots from `bench/golden/`
- **Zenodo DOI** per release tag
- **`papers/<venue>/`** directory frozen at submission

Failed approaches are logged in [`CHANGELOG.md`](CHANGELOG.md) under the
"Failed Approaches" heading (eight entries currently); this is unusual project
hygiene and is referenced in the artifact submission ([ADR-0014](docs/adr/0014-failed-approach-tracking.md)).

## Citing QFault

A formal citation will accompany the v0.1.0 arXiv preprint (target Q4 2026).
For now, please cite this repository:

```bibtex
@software{qfault_2026,
  author       = {Sri-Harsha-T},
  title        = {QFault: A C++20 Surface-Code-First Fault-Tolerant Quantum Compiler},
  year         = {2026},
  url          = {https://github.com/Sri-Harsha-T/qfault_cc},
  note         = {v0.1.0-stage2}
}
```

## License

Apache License 2.0 — see [`LICENSE`](LICENSE) and [ADR-0015](docs/adr/0015-apache-license.md).
Drop-in compatible with Qiskit, Stim, CUDA-Q, PennyLane, and TKET.

## Contributing

Contributions follow the CCPM workflow ([`.claude/skills/ccpm`](.claude/skills/ccpm)):
PRD → epic → tasks → GitHub issues → parallel agent execution → exit reports.
Before opening a PR, please:

1. Read the relevant ADR(s) — they record decisions that should not be re-opened
   without a new ADR
2. Run `./scripts/quick-test.sh` (must pass in < 60 seconds)
3. Ensure `clang-tidy` and ASAN+UBSAN are clean on changed files
4. Update `CHANGELOG.md` (and "Failed Approaches" if any approach was tried and
   abandoned)

See [CONTRIBUTING.md](CONTRIBUTING.md) for the full process.

## Acknowledgements

QFault builds on, wraps, or validates against the following open-source tools:

- [Stim](https://github.com/quantumlib/Stim) (Apache 2.0) — stabilizer simulator and equivalence oracle
- [MQT QCEC](https://github.com/munich-quantum-toolkit/qcec) (MIT) — quantum circuit equivalence checking
- [GridSynth / newsynth](https://github.com/kenmcken/newsynth) (BSD) — Ross-Selinger T-gate synthesis
- [GoogleTest](https://github.com/google/googletest) (BSD-3) — unit testing
- [Litinski 2019, *A Game of Surface Codes*](https://arxiv.org/abs/1808.02892) — lattice surgery layout templates
- [Beverland 2022, *Assessing requirements to scale to practical quantum advantage*](https://arxiv.org/abs/2211.07629) — MSD factory cost models
- [Silva et al. 2024, *Multi-qubit Lattice Surgery Scheduling*](https://arxiv.org/abs/2405.17688) — earliest-available-first scheduling

Full citations are recorded in the ADRs and in the upcoming arXiv preprint.
