# ADR-0003: Global Code Distance d for v0.1; Variable-d Deferred

**Status:** Accepted  
**Date:** 2026-04-24  
**Supersedes:** —

---

## Context

Surface code error protection is parameterised by code distance `d`. In a real
fault-tolerant computer, different logical qubits or circuit regions may use
different code distances (higher d where lower error rates are needed, lower d
for ancilla-heavy regions to save qubit overhead). This is "variable-d compilation."

---

## Decision

**v0.1 uses a single, global code distance `d` as a compiler parameter.**
All patches in the compiled output use the same `d`.
Variable-d compilation is deferred to v0.2.

---

## Alternatives Considered

| Alternative | Why Deferred |
|-------------|-------------|
| Variable-d compilation | Dramatically complicates the routing pass (patches of different sizes don't tile uniformly); complicates ResourceEstimator (heterogeneous qubit counts); complicates MSD scheduling (factories may need to serve qubits at different d). The engineering complexity is a v0.2 item. |

---

## Consequences

**Positive:** Dramatically simplifies routing, ResourceEstimator, and MSD scheduling.

**Negative:** The compiler produces suboptimal qubit counts vs. a variable-d compiler.
This **must be stated clearly as a limitation** in the arXiv preprint and in
`ResourceReport` output ("Note: global d assumption; variable-d would reduce footprint").

**Risks:** Academic reviewers will flag this. The mitigation is to document it
explicitly as a known limitation with a roadmap item.

---

## Implementation Notes for AI Sessions

- `d` is a parameter in `PassContext`, set at pipeline construction time
- ALL passes read `d` from `PassContext` — never hardcode it
- `ResourceReport` must include a `"global_d_assumption": true` field in JSON output
- The limitation must appear in the `--help` output of any CLI tool
