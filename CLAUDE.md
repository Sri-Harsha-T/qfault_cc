# QFault — Fault-Tolerant Quantum Compiler (C++20)

QFault is an open-source, modular compiler-pass library that takes a logical
quantum circuit and compiles it to fault-tolerant gate sequences for surface
code execution. It outputs OpenQASM 3.0 or QIR-compatible bytecode.

Target audience: quantum systems engineers and QEC researchers who currently
glue 4–5 tools (Stim, GridSynth, PyLIQTR, newsynth) manually.

## How to navigate this repo — read in this order each session

1. **This file** — conventions and commands
2. `memory-bank/activeContext.md` — current stage, active story, next action
3. `CHANGELOG.md` → section "Failed Approaches" — do NOT retry listed approaches
4. On demand: `docs/architecture.md`, `docs/glossary.md`, `docs/adr/README.md`

## Build, test, lint

```bash
./scripts/quick-test.sh        # <60s — run before every commit
./scripts/full-build.sh        # full CMake build, both clang and gcc
./scripts/asan-ubsan.sh        # address + undefined-behaviour sanitizers
./scripts/compare-stim.sh      # reference diff vs Stim oracle
./scripts/benchmark.sh         # T-count and gate-count benchmarks
```

CMake minimum: 3.16. Compilers: clang-17+ and gcc-13+ must both build clean.

## Coding rules (enforced — use hooks not just memory)

- C++20 throughout; Concepts over inheritance hierarchies
- No raw `new`/`delete`; RAII; follow C++ Core Guidelines
- No `std::variant` for payload types in hot paths — causes compile-time blowup
- Interfaces via Concept + free-function dispatch, not virtual
- Never commit code that breaks a passing test
- Run `./scripts/quick-test.sh` before every commit
- `clang-tidy` and `cppcheck` must be clean on changed files
- ASAN + UBSAN must be clean on changed files

## Architecture pointers (load only when relevant)

- IR & pass manager design: `docs/architecture.md`
- QEC domain terms (stabilizer, syndrome, code distance, Pauli frame…): `docs/glossary.md`
- All architectural decisions: `docs/adr/README.md` (index table)
- Surface code + lattice surgery specifics: `docs/architecture.md#lattice-surgery`
- Path-scoped rules: `.claude/rules/cpp.md`, `.claude/rules/qec.md`

## Session discipline

- Update `memory-bank/activeContext.md` → "Next Action" before ending any session
- Log failed approaches to `CHANGELOG.md` under "## Failed Approaches"
- Propose an ADR (`/adr`) before: adding a dependency, changing a numerical
  convention, switching synthesis algorithms, or changing the IR schema
- One `in_progress` TodoWrite item at a time
- `/compact` at ~60% context fill with a focus directive
- `/clear` between unrelated tasks — write state to `activeContext.md` first

## ccpm project management (in .claude/skills/ccpm)

Use ccpm slash commands for all ticket and epic work:

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
