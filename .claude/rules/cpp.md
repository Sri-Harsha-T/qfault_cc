---
paths:
  - "**/*.cpp"
  - "**/*.hpp"
  - "**/*.cc"
  - "**/*.h"
---

# C++ Code Rules for QFault

These rules apply to all C++ source files.

## Language

- C++20 exclusively. Use Concepts, `std::span`, ranges, designated initialisers.
- No C++17 workarounds unless forced by a library dependency.
- `[[nodiscard]]` on all factory functions and non-void pure query methods.
- `[[likely]]` / `[[unlikely]]` only in measured hot paths.

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
- Virtual dispatch in the inner instruction-processing loop.
- `#pragma once` is acceptable; prefer include guards for portability.

## Testing

- Every new class gets a unit test file `tests/unit/test_<classname>.cpp`.
- Test names follow `TEST(ClassName, DescribesBehaviour)` — descriptive, not generic.
- GoogleTest only — no hand-rolled test frameworks.
- Test coverage ≥80% on changed lines (enforced in CI via gcov).
