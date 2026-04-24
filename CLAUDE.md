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

> **Status:** CMakeLists.txt not yet written (Stage 1, Epic 1-D). Scripts below are
> stubs. `scripts/benchmark.sh` does not exist yet — do not reference it.

Once CMakeLists.txt exists, use these cmake preset commands:
```bash
cmake --preset debug                   # configure debug build
cmake --build build/debug -j           # build
ctest --test-dir build/debug -j        # run all tests

cmake --preset release && cmake --build build/release -j
cmake --preset asan && cmake --build build/asan -j  # address + UB sanitizers
```

Pre-commit and CI wrappers (will be filled in during 1-D):
```bash
./scripts/quick-test.sh        # <60s — run before every commit
./scripts/full-build.sh        # full CMake build, both clang and gcc
./scripts/asan-ubsan.sh        # address + undefined-behaviour sanitizers
./scripts/compare-stim.sh      # reference diff vs Stim oracle
```

Static analysis (run on changed files before any PR):
```bash
clang-tidy --config-file=.clang-tidy src/
cppcheck --enable=all --suppress=missingInclude src/
```

CMake minimum: 3.16. Compilers: clang-17+ and gcc-13+ must both build clean.

## Source directory layout

```
src/qfault/
  ir/           # QFaultIR data structures (Stage 1)
  passes/       # PassManager + all compiler passes
    synthesis/  # SynthesisProvider Concept + GridSynth/SK providers (Stage 2)
    lattice/    # LatticeSurgeryPass (Stage 3)
    msd/        # MSDSchedulerPass (Stage 4)
    estimate/   # ResourceEstimatorPass (Stage 4)
  frontend/     # QASM 3.0 parser (Stage 1, subset)
  backend/      # QASM 3.0 + QIR emitters (Stage 5)
  util/         # Logging, error types, diagnostics
include/qfault/ # Public headers only (pimpl where appropriate)
tests/
  unit/         # Per-class GoogleTest files (test_<classname>.cpp)
  integration/  # End-to-end circuit → QIR → Stim round-trips
  benchmarks/   # Google Benchmark (gated by QFAULT_RUN_BENCHMARKS=ON)
  reference/    # Committed golden Stim outputs — diff on CI, never modify without review
bindings/python/ # pybind11 bindings (Stage 5 only)
```

## Active stage

**Stage 1 of 5: IR + Pass Manager Core** (`docs/phases/stage-1-ir-pass-manager/`)
- Spec (epics, stories, ACs, prompt plan): `docs/phases/stage-1-ir-pass-manager/spec.md`
- Daily todo tracker: `docs/phases/stage-1-ir-pass-manager/todo.md`
- Execution order: **1-D (build system) first**, then 1-A (IR types), 1-B (PassManager), 1-C (parser)

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
- MSD factory footprint (15-to-1): approximately `(4d+1) × (8d+1)` data patches
- MERGE always pairs matching boundaries (X↔X or Z↔Z) — mixing is a logic error, add assertion
- GridSynth is the default synthesis provider (ADR-0004) — do NOT change this default
- Synthesis ε = 1e-10 by default — read from `PassContext`, never hardcode

## Architecture pointers (load only when relevant)

- IR & pass manager design: `docs/architecture.md`
- QEC domain terms: `docs/glossary.md`
- All architectural decisions (index): `docs/adr/README.md`
- Surface code + lattice surgery: `docs/architecture.md#lattice-surgery`
- Path-scoped rules: `.claude/rules/cpp.md`, `.claude/rules/qec.md`

## Decided ADRs (do not re-open without a new ADR)

| ADR | Decision |
|-----|----------|
| ADR-0001 | Two-level IR: single `QFaultIRModule` with `variant<LogicalGate, PatchOp>` |
| ADR-0002 | `SynthesisProvider` via C++20 Concept, not virtual base |
| ADR-0003 | Global code distance `d` for v0.1; variable-d deferred to v0.2 |
| ADR-0004 | GridSynth as default synthesiser; SK as benchmark baseline only |

ADR-0005 (QIR spec pinning) and ADR-0006 (routing algorithm) are still **Draft** — write them before Stage 5 and Stage 3 respectively.

## Session discipline

- Update `memory-bank/activeContext.md` → "Next Action" before ending any session
- Log failed approaches to `CHANGELOG.md` under "## Failed Approaches"
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
