# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## What this repo is

QFault is an open-source, modular C++20 compiler-pass library that takes a logical
quantum circuit (Clifford+T) and compiles it to fault-tolerant gate sequences for
surface code execution, outputting OpenQASM 3.0 or QIR bytecode.

Target audience: QEC researchers and quantum systems engineers who currently
glue 4–5 tools (Stim, GridSynth, PyLIQTR, newsynth) manually.

**GitHub:** `git@github.com:Sri-Harsha-T/qfault_cc.git`

## How to navigate this repo — read in this order each session

1. **This file** — conventions and commands
2. `memory-bank/activeContext.md` — current stage, active story, next action
3. `CHANGELOG.md` → section "Failed Approaches" — do NOT retry listed approaches
4. On demand: `docs/architecture.md`, `docs/glossary.md`, `docs/adr/README.md`

## Build, test, lint

> **Status (post-v0.2 audit, 2026-04-26):** Stages 1+2 shipped. Stage 3 active.
> Stage 2.5 (Verify+Bench), Stage 6 (Native Synthesis), Stage 7 (Formal/MLIR
> Stretch) added per audit. See ROADMAP.md and `docs/adr/README.md`.

Cmake preset commands:
```bash
cmake --preset gcc13-debug                    # configure debug build
cmake --build build/gcc13-debug -j            # build
ctest --test-dir build/gcc13-debug -j         # run all tests

cmake --preset clang18-release && cmake --build build/clang18-release -j
cmake --preset clang18-asan    && cmake --build build/clang18-asan -j
```

Pre-commit and CI wrappers:
```bash
./scripts/quick-test.sh        # <60s — run before every commit
./scripts/full-build.sh        # full CMake build, both clang and gcc
./scripts/asan-ubsan.sh        # address + undefined-behaviour sanitizers
./scripts/compare-stim.sh      # Stim oracle (Stage 2.5; stub through Stage 2)
./scripts/qcec-equivalence.sh  # MQT QCEC equivalence check (Stage 2.5)
./scripts/bench-synthesis.sh   # Stage 2 gate benchmark (overhead ≤5%)
```

Static analysis (run on changed files before any PR):
```bash
clang-tidy --config-file=.clang-tidy src/
```

CMake minimum: 3.21 (presets). Local install: `~/.local/bin/cmake` (pip). Scripts auto-detect it.
Compilers: clang-18 and gcc-13 must both build clean (CI); gcc-9 is insufficient.

## Source directory layout

```
src/qfault/
  ir/           # QFaultIR data structures (Stage 1)
  passes/       # PassManager + all compiler passes
    synthesis/  # SynthesisProvider Concept + GridSynth/BFS-table providers (Stage 2)
                # + native Ross-Selinger / Kliuchnikov providers (Stage 6)
    lattice/    # LatticeSurgeryPass: A* router + Litinski templates (Stage 3)
    msd/        # MSDSchedulerPass with Beverland 2022 catalog (Stage 4)
    estimate/   # ResourceEstimatorPass (Stage 4)
    optimise/   # PhasePolyZXPass (post-v0.1, ADR-0011 Draft)
  frontend/     # QASM 3.0 parser (Stage 1, subset)
  backend/      # QASM 3.0 + QIR + Stim native emitters (Stage 5)
  util/         # Logging, error types, diagnostics
  verify/       # Stim + QCEC harness wrappers (Stage 2.5)
include/qfault/ # Public headers only (pimpl where appropriate)
mlir/           # qfault.fto MLIR dialect (Stage 7 Option A only)
proofs/         # Coq / Rocq sources (Stage 7 Option B only)
tests/
  unit/         # Per-class GoogleTest files (test_<classname>.cpp)
  integration/  # End-to-end circuit → QIR → Stim round-trips
  benchmarks/   # Google Benchmark (gated by QFAULT_RUN_BENCHMARKS=ON)
  reference/    # Committed golden Stim outputs — diff on CI, never modify without review
bindings/python/ # pybind11 bindings (Stage 5 only; ADR-0012)
papers/         # arXiv source + figures (Stage 5; per ADR-0017)
```

## Active stage

**Stage 3 of 7: Lattice Surgery Mapper** (`docs/phases/stage-3-lattice/`).
Stages 1+2 complete; Stage 2.5 / 6 / 7 are post-Stage-3 / post-Stage-5
deliverables added in the v0.2 audit.

## Two-level IR — central invariant

`QFaultIRModule` holds an `IRLevel` tag and a `std::vector<std::variant<LogicalGate, PatchOp>>`.
Every pass **must** call `module.assertLevel(requiredLevel())` at the start of `run()`.
The `LatticeSurgeryPass` is the only pass that transitions the module from `LOGICAL` → `PHYSICAL`.
See ADR-0001 and `docs/architecture.md` for the full design rationale.

## Coding rules (enforced — use hooks not just memory)

- C++20 throughout; Concepts over inheritance hierarchies
- No raw `new`/`delete`; RAII; follow C++ Core Guidelines
- No `std::variant` for payload sub-types in hot paths — causes compile-time blowup
- Interfaces via Concept + free-function dispatch, not virtual
- `enum class` for all enumerations — no plain `enum`
- No `using namespace std;` anywhere
- No exceptions in the hot path — use `std::expected<T, QFaultError>` for fallible ops
- Log diagnostics via `PassContext::addDiagnostic()`, never `std::cerr` directly
- `[[nodiscard]]` on all factory functions and pure query methods
- Every new class → `tests/unit/test_<classname>.cpp`; test names: `TEST(ClassName, DescribesBehaviour)`

## QEC invariants (apply everywhere)

- Code distance `d` must always be **odd and ≥ 3** — `assert(d % 2 == 1 && d >= 3)`
- Pauli enum: `I=0, X=1, Y=2, Z=3`
- Rotated surface code is the only target for v0.1 (not toric, not colour code)
- Physical qubits per patch at distance d: `2d² − 1` (d² data + d²−1 measure)
- BV-10 d=5 reference: 11 logical × (2d²−1) = 539 base + routing → 600–800 physical
- MSD factory footprint (Beverland 2022 Table VII): see ADR-0007
- MERGE always pairs matching boundaries (X↔X or Z↔Z) — mixing is a logic error, add assertion
- GridSynth is the default synthesis provider (ADR-0004) — do NOT change this default,
  but be aware it is a `popen` wrapper around the Haskell binary in v0.1 (ADR-0013)
- Synthesis ε = 1e-10 by default — read from `PassContext`, never hardcode
- Litinski 2019 layout templates: compact 1.5n+3 / 9τ; intermediate 2n+4 / 5τ
  (Fig 13a says 2.5n+4; the body says 2n+4 — use the body value); fast 2n+√(8n)+1 / 1τ
- Stim library target is `libstim` (NOT `stim`); `SIMD_WIDTH=64` for reproducibility
- liblsqecc is GPL-3.0 — incompatible with our Apache 2.0 (ADR-0015); do NOT vendor it

## Architecture pointers (load only when relevant)

- IR & pass manager design: `docs/architecture.md`
- QEC domain terms: `docs/glossary.md`
- All architectural decisions (index): `docs/adr/README.md`
- Surface code + lattice surgery: `docs/architecture.md#lattice-surgery`
- Path-scoped rules: `.claude/rules/cpp.md`, `.claude/rules/qec.md`

## Decided ADRs (do not re-open without a new ADR)

| ADR | Status | Decision |
|-----|--------|----------|
| ADR-0001 | Accepted | Two-level IR: single `QFaultIRModule` with `variant<LogicalGate, PatchOp>` |
| ADR-0002 | Accepted | `SynthesisProvider` via C++20 Concept, not virtual base |
| ADR-0003 | Accepted | Global code distance `d` for v0.1; variable-d deferred to v0.2 |
| ADR-0004 | Accepted (with limitation) | GridSynth as default synthesiser; superseded in part by ADR-0013 |
| ADR-0005 | Accepted | QIR Alliance v0.1 base profile pinning |
| ADR-0006 | Accepted | Lattice surgery routing: A\* + Litinski templates + Silva 2024 EAF |
| ADR-0007 | Accepted | MSD factory selection: catalog enumeration with Beverland 2022 cost formulas |
| ADR-0008 | Accepted | Synthesis algorithm portfolio: Ross-Selinger now, Kliuchnikov-2023 in Stage 6 |
| ADR-0009 | Accepted | Verification strategy: Stim + MQT QCEC, framed as **validation** not verification |
| ADR-0010 | Accepted | Output backend portfolio: QASM 3.0 + QIR + Stim native |
| ADR-0011 | Draft | Phase-polynomial / ZX pass: native AMM + optional PyZX bridge |
| ADR-0012 | Accepted | Python bindings: pybind11 v2.11.1 for v0.1; revisit nanobind for v0.2 |
| ADR-0013 | Accepted | SKProvider reframed as fallback / sanity oracle (rename to BFSTableProvider) |
| ADR-0014 | Accepted | Failed-approach tracking is project policy |
| ADR-0015 | Accepted | License: Apache 2.0 with explicit patent grant |
| ADR-0016 | Accepted | Conference target ladder: QCE26 → CGO27 → OOPSLA / PLDI gated on Stage 7 |
| ADR-0017 | Accepted | Reproducibility infrastructure: Dockerfile + flake.nix + Zenodo + papers/ |
| ADR-0018 | Draft | MLIR `qfault.fto` dialect (Stage 7 Option A) |

## Session discipline

- Update `memory-bank/activeContext.md` → "Next Action" before ending any session
- Log failed approaches to `CHANGELOG.md` under "## Failed Approaches" (per ADR-0014)
- Propose an ADR (`/adr`) before: adding a dependency, changing a numerical
  convention, switching synthesis algorithms, or changing the IR schema
- One `in_progress` TodoWrite item at a time
- `/compact` at ~60% context fill with a focus directive
- `/clear` between unrelated tasks — write state to `activeContext.md` first

## ccpm project management (in .claude/skills/ccpm)

```
/pm:prd-new           Create a new PRD document for a stage
/pm:prd-parse         Parse PRD → epic + story structure
/pm:epic-decompose    Break epic into GitHub Issues (stories + tasks)
/pm:issue-start <id>  Checkout worktree and begin issue
/pm:next              Get the next prioritised unblocked story
/pm:standup           Daily status across all open issues
```

## Definition of Done (every story)

- [ ] All AC tests pass (`ctest -j` green on clang AND gcc)
- [ ] No new `-Wall -Wextra -Wpedantic` warnings
- [ ] ASAN + UBSAN clean on changed files
- [ ] `clang-tidy` clean on changed files
- [ ] Test coverage on changed lines ≥ 80%
- [ ] `CHANGELOG.md` updated (new entry or "Failed approaches" if relevant)
- [ ] PR description references GitHub Issue number
- [ ] `memory-bank/activeContext.md` "Next Action" updated
