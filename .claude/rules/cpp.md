---
paths:
  - "**/*.cpp"
  - "**/*.hpp"
  - "**/*.cc"
  - "**/*.h"
---

# C++ Code Rules for QFault

These rules are loaded automatically when Claude Code touches any C++ file.
They make explicit what `CLAUDE.md` says abstractly. The `cpp-pro` agent uses these as its primary checklist.

## Language

- C++20 exclusively. Use Concepts, `std::span`, ranges, designated initialisers.
- No C++17 workarounds unless forced by a library dependency.
- `[[nodiscard]]` on all factory functions and non-void pure query methods.
- `[[likely]]` / `[[unlikely]]` only in measured hot paths.
- **`static_assert` for new Concepts** in the corresponding test file.
- **Test for every new public class/free function/template.** Test name pattern:
  `TEST(ClassName, DescribesBehaviour)`.
- **`enum class` with explicit underlying type** when the values cross an ABI:
  `enum class GateKind : std::uint8_t { ... }`.

## Memory and Ownership

- No raw `new` / `delete`. Use `std::make_unique` / `std::make_shared`.
- Prefer stack allocation and value semantics for small QFaultIR nodes.
- `std::span<const T>` for non-owning array views, not raw pointers.
- No `shared_ptr` in hot loops (instruction processing) — use references or indices.

## Type Safety

- No C-style casts. Use `static_cast`, `std::bit_cast`, or Concepts.
- `enum class` for all enumerations — no plain `enum`.
- No implicit narrowing conversions.

## Templates and Concepts

- Constrain all templates with Concepts — no unconstrained `typename T`.
- `static_assert(SynthesisProvider<T>)` in test files for each provider.
- Do NOT use `std::variant` for hot-path payload subtypes — use Concept dispatch.

## Error Handling

- No exceptions in the hot path (synthesis inner loop, pass runner).
- Use `std::expected<T, QFaultError>` for fallible operations (C++23 idiom,
  or a local `Expected<T,E>` shim if targeting C++20 only).
- Log diagnostics through `PassContext::addDiagnostic()` — never `std::cerr` directly.

## Forbidden Patterns

- `using namespace std;` — banned globally.
- `std::variant` in `LogicalGate` or `PatchOp` payload subtypes — causes
  compile-time blowup (see CHANGELOG.md "Failed Approaches").
- Virtual dispatch in inner loops. Virtual at pass granularity (`PassBase`)
  is fine; per-gate or per-tile virtual dispatch is rejected.
- `#pragma once` is acceptable; prefer include guards for portability.
- Raw `new` / `delete` / `malloc` / `free`. RAII via `std::make_unique` /
  `std::make_shared` only.
- Plain `enum`. Use `enum class` always.
- `std::cerr` / `std::cout` / `printf` outside `main()` and `dump()` methods.
  All diagnostic output flows through `PassContext::addDiagnostic()`.
- Exceptions in hot paths. Use `tl::expected<T, QFaultError>` (the C++23
  `std::expected` shim) for fallible operations.
- Boost. Period. (We pin a small dependency surface.)
- Raw `T*` + `size_t` for non-owning array views. Use `std::span<const T>`.
- `std::vector<bool>`. Use `std::vector<uint8_t>` for bitmaps.

## Testing

- Every new class gets a unit test file `tests/unit/test_<classname>.cpp`.
- Test names follow `TEST(ClassName, DescribesBehaviour)` — descriptive, not generic.
- GoogleTest only — no hand-rolled test frameworks.
- Test coverage ≥80% on changed lines (enforced in CI via gcov).

# Sanitizer-clean patterns (logged from Failed Approaches)

- Use **signed integers for arithmetic that might overflow** — UBSAN catches
  signed overflow; unsigned wraps silently. A* costs are `int64_t`, not `size_t`.
- Use **`std::vector<uint8_t>`** as a bitmap, not `std::vector<bool>`. ASan
  reports false positives on the proxy type; iterators don't satisfy iterator
  concepts cleanly.
- **`std::span<const T>`** for non-owning array views. Survives sanitizer
  instrumentation cleanly.
- **Pin SIMD width** across translation units when integrating Stim. Stim's
  `MAX_BITWORD_WIDTH` template parameter must match across all TUs or you get
  ODR-violation crashes. Use `-DSIMD_WIDTH=64` for golden test reproducibility.

## C++20 ranges — what works, what doesn't

Available in C++20 (use freely): `views::filter`, `transform`, `take`, `drop`,
`reverse`, `iota`, `join`, `split`, `elements`.

**Missing in C++20** (need C++23, range-v3, or manual loop):
`views::enumerate`, `views::zip`, `views::chunk`, `views::stride`,
`std::ranges::to`.

**Broken in older toolchains:** `views::join` over views-of-views fails on
GCC ≤ 10, Clang ≤ 14, MSVC ≤ 16.9. **Project requires GCC 13+ or Clang 18+**;
these are fine.

**Materialization rule:** Treat each pass as returning a materialized
`std::vector` for testability. Use ranges *inside* a pass for laziness; never
store a view past its underlying container's lifetime — that's silent UB
invisible to ASan.

## Concepts patterns

```cpp
// In a header:
template <typename T>
concept SynthesisProvider = requires(T p, double angle, double eps) {
    { p.synthesise(angle, eps) } -> std::convertible_to<std::vector<GateKind>>;
    { p.name() }                 -> std::convertible_to<std::string_view>;
};

// In tests/unit/test_SynthesisProvider.cpp:
static_assert(SynthesisProvider<GridSynthProvider>);
static_assert(SynthesisProvider<BFSTableProvider>);
static_assert(SynthesisProvider<MockProvider>);
static_assert(!SynthesisProvider<int>);
static_assert(!SynthesisProvider<NotAProvider>);
static_assert(!SynthesisProvider<MissingName>);
```

The static_assert pattern catches API drift at compile time. Concept failure
messages are still verbose; the explicit asserts give readable diagnostics.

## std::variant + std::visit pattern (ADR-0001)

```cpp
template <class... Ts> struct overload : Ts... { using Ts::operator()...; };
template <class... Ts> overload(Ts...) -> overload<Ts...>;

std::visit(overload{
    [&](const LogicalGate& g) { /* ... */ },
    [&](const PatchOp& op)    { /* ... */ },
}, instr);
```

Always use `overload{}`, never `if (std::holds_alternative<...>)` chains.
`std::visit` typically compiles to a contiguous jump table.

## Test naming (enforced by reviewer)

```cpp
// GOOD — describes behaviour, names the SUT
TEST(PassManager, RunsPassesInRegistrationOrder) { ... }
TEST(GridSynthProvider, ReturnsEmptyVectorWhenBinaryAbsent) { ... }
TEST(SKProvider, ReturnsExactTForRzPiOver4) { ... }

// REJECTED
TEST(Test1, Works) { ... }
TEST(MyTest, DoesAThing) { ... }
TEST(PassManagerTest, TestRun) { ... }
```

## Build presets

- **Use named presets always**: `gcc13-debug`, `gcc13-release`, `clang18-debug`,
  `clang18-release`, `clang18-asan`, `coverage`. Never plain `debug` or
  `release` (they pick up system compiler, which may be gcc-9.4 — logged Failed
  Approach 2026-04-25).

## Static analysis

- `clang-tidy --config-file=.clang-tidy` clean on changed files
- `cppcheck --enable=all --suppress=missingInclude` clean on changed files
- No NOLINT suppressions without an explicit comment justifying why and a
  CHANGELOG entry.