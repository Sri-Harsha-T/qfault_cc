# Stage 1 Exit Report — IR + Pass Manager Core

**Stage:** 1 of 5
**Status:** COMPLETE
**Completed:** 2026-04-25
**Duration:** 1 day (2026-04-24 → 2026-04-25) — accelerated by AI-assisted development

---

## Stage Gate Result: PASSED

Integration test `tests/integration/test_ir_two_level.cpp` confirms the two-level IR
works correctly. ADR-0001 is now Accepted.

---

## Test Count at Gate

| Preset | Tests | Result |
|--------|-------|--------|
| gcc13-debug | 93 | ✅ Green |
| gcc13-release | 93 | ✅ Green |
| clang18-debug | 93 | ✅ Green |
| clang18-release | 93 | ✅ Green |
| clang18-asan | 93 | ✅ Green (after UBSAN fix) |

---

## Deliverables Shipped

| Item | Commit | Notes |
|------|--------|-------|
| CMakeLists.txt + CMakePresets.json | b262a63 | 9 presets; GoogleTest via FetchContent |
| .clang-tidy, .clang-format, scripts/quick-test.sh | 2503817 | All IR types + passes |
| LogicalGate, PatchOp, QFaultIRModule, PassManager core | 30b4142 | IR + PassManager skeleton |
| Stage gate integration test | 1b05ca4 | ADR-0001 confirmed |
| PassManager add<T>()/run() | dd025ac | Type-safe pass chaining |
| NoOpPass + round-trip integration test | fc8bdad | End-to-end pass pipeline |
| PassManager::printStats() | d081444 | Per-pass timing output |
| QASM 3.0 Lexer + Parser + round-trip test | 6ef5bd4 | Frontend complete |
| GitHub Actions CI matrix | df0c6ce | 4-way matrix + ASAN job |

---

## ADR Status at Stage Close

| ADR | Status | Decision |
|-----|--------|----------|
| ADR-0001 | ✅ Accepted | Two-level IR: `variant<LogicalGate, PatchOp>` — viable, no structural incompatibility |
| ADR-0002 | Draft | `SynthesisProvider` via C++20 Concept |
| ADR-0003 | Draft | Global code distance `d` for v0.1 |
| ADR-0004 | Draft | GridSynth as default synthesiser |

ADR-0002, -0003, -0004 must be finalized at the start of Stage 2.

---

## Issues Found and Resolved

| Issue | Root Cause | Fix |
|-------|------------|-----|
| UBSAN signed integer overflow in `test_PassContext::TimerMeasuresElapsedTime` | `volatile int sum` accumulates 0..99999; sum(0..65535) = 2147450880 exceeds INT_MAX when adding i=65536 | Changed to `volatile long long sum` |

---

## Lessons Learned

1. **gcc-13 `-Wmissing-field-initializers`** fires on designated-init aggregates that omit fields even when the default is correct. Fix: add `= std::nullopt` / `= {}` as default member initializers in the struct itself. (Logged in CHANGELOG.md Failed Approaches.)

2. **`/*comment*/` as range-for loop variable** is invalid C++ — a comment is not an identifier. Use a named variable + `(void)var;`. (Logged in CHANGELOG.md Failed Approaches.)

3. **`= default` operator==** requires C++20 — the system default compiler (gcc 9.4 on this dev machine) doesn't support it. Always use named presets (`gcc13-debug`, `clang18-debug`) rather than the generic `debug` preset locally.

4. **clang-tidy-18** is not transitively installed with `clang-18` on ubuntu-24.04 — must be explicitly listed in apt-get.

---

## Stage Gate Sign-off

- ✅ All 93 tests green on clang-18 and gcc-13 (debug + release + asan)
- ✅ ASAN + UBSAN clean (after overflow fix)
- ✅ clang-tidy clean on all `src/` files
- ✅ ADR-0001 status: Accepted
- ✅ GitHub Milestone "Stage 1: IR + Pass Manager Core" closed
- ✅ `memory-bank/activeContext.md` updated with Stage 2 next action
- ✅ `CHANGELOG.md` updated

**Next:** Stage 2 — Synthesis Pass (T-Gate). See `docs/phases/stage-2-synthesis/spec.md`.
