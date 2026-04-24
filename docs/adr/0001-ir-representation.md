# ADR-0001: IR Representation — Two-Level in One Module

**Status:** Accepted  
**Date:** 2026-04-24  
**Supersedes:** —

---

## Context

QFault needs to represent quantum circuits at two levels:
1. **Logical level:** Abstract gates (H, CNOT, T, S) over named logical qubits
2. **Physical level:** Surface code patch operations (MERGE, SPLIT, MEASURE)
   over 2D patch grid coordinates

Both levels must be processed by the PassManager. The question is whether
they live in a single `QFaultIRModule` or in two separate module types with
an explicit lowering boundary.

---

## Decision

Use a **single `QFaultIRModule`** containing a `std::vector<Instruction>` where
`Instruction = std::variant<LogicalGate, PatchOp>`. The module carries an
`IRLevel` tag (`LOGICAL` or `PHYSICAL`). Passes assert the correct level before
running. The `LatticeSurgeryPass` transitions the module from `LOGICAL` to
`PHYSICAL` in place.

---

## Alternatives Considered

| Alternative | Why Rejected |
|-------------|-------------|
| Two separate module types with an explicit lowering pass boundary | Doubles the IR API surface. Every utility (printing, analysis, testing) must be implemented twice. Complicates pass pipeline — passes can't easily peek across levels. |
| Single module with `union` (C-style) | Unsafe, not compatible with C++20 idioms |
| Base class with virtual dispatch per instruction | Virtual per-instruction dispatch is too slow for millions-of-gate circuits |

---

## Consequences

**Positive:**
- One IR type → one PassManager → simpler pipeline code
- Passes can inspect both levels simultaneously if needed (e.g., hybrid passes)
- One printing/serialisation implementation

**Negative / Trade-offs:**
- The `IRLevel` tag is a runtime invariant, not a compile-time type constraint
- A logical-level pass invoked on a physical-level module will assert at runtime,
  not compile time — requires disciplined pass ordering

**Risks:**
- If the two levels require structurally incompatible representations (e.g., logical
  gates need symbolic qubit names but physical ops need numeric grid coords),
  the discriminated union approach may need to be revisited
- **Stage 1 gate explicitly checks this risk** before committing to the approach

---

## Implementation Notes for AI Sessions

- `IRLevel` should be an `enum class`: `enum class IRLevel { LOGICAL, PHYSICAL };`
- Each pass must call `module.assertLevel(requiredLevel())` at the start of `run()`
- The `variant` should be `std::variant<LogicalGate, PatchOp>` — do NOT use
  inheritance or base pointers
- Do NOT use `std::variant` for payload sub-types within `LogicalGate` or `PatchOp`
  if those payload types appear in hot loops — this was a known failure mode
  (see CHANGELOG.md "Failed Approaches" when first encountered)
- Visitor pattern for printing: `std::visit(overload{...}, instruction)`

---

## References

- Fowler et al., "Surface codes: Towards practical large-scale quantum computation" (2012)
- LLVM Language Reference (for IR design inspiration)
- `memory-bank/systemPatterns.md` § "Two-Level IR"
