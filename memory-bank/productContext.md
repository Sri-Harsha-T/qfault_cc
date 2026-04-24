# Product Context — QFault

## Why This Project Exists

The quantum computing industry is entering the "utility era" where fault-tolerant
demonstrations are actively being pursued by Google (Willow), IBM (Heron), and
Microsoft (Majorana 1). The hardware bottleneck is shifting: **the new bottleneck
is compilation infrastructure**.

Google's open quantum compiling roles explicitly list "compiling logical quantum
algorithms to low-level fault-tolerant instructions" as the core task. This is
a funded, active need — and the open-source tooling to support it is fragmentary.

## Primary Users

**User A — QEC Researcher**
- Wants to compile a new quantum algorithm (e.g., phase estimation variant)
  to surface code instructions quickly, without mastering 4 separate tools
- Values: correctness guarantees, resource estimates for grant writing, Python API
- Frustration: each pipeline change requires updating glue scripts across 4 repos

**User B — Quantum Systems Engineer (industry)**
- Benchmarks compilation strategies across different synthesis algorithms
- Values: C++ performance, pluggable synthesiser interface, QIR output
- Frustration: no single tool produces valid FT instruction streams at scale

**User C — Hardware Group (internal tooling)**
- Needs resource estimates (physical qubits, T-factory count, time-steps)
  that reflect actual routing decisions, not heuristic approximations
- Values: tight estimates, auditable compilation pipeline, open source

## Competitive Positioning

| Tool | Their Strength | QFault Advantage |
|------|---------------|-----------------|
| Stim | Blazing simulation | We compile; they simulate. Complementary. |
| GridSynth | T-optimal synthesis | We *wrap* GridSynth; build on it, don't replace |
| PyLIQTR | Resource estimation | Our estimates are compiler-derived, not heuristic |
| Qiskit QEC | Ecosystem size | We are faster (C++) and FT-aware by default |
| CUDA-Q | MLIR infra | We are open-source, vendor-neutral |

## Key Value Propositions

1. **One tool, full pipeline:** logical circuit → scheduled FT instruction stream
2. **Pluggable synthesis:** swap GridSynth ↔ Solovay-Kitaev without code changes
3. **Spatial routing awareness:** MSD factories treated as physical resources
4. **Compiler-derived resource estimates:** not heuristic approximations
5. **Open source (Apache 2.0):** auditable, forkable, community-extensible
6. **Python-accessible:** pybind11 bindings for Jupyter/research workflows

## Positioning Statement

> For quantum researchers and engineers who need to compile logical quantum
> circuits to fault-tolerant surface code instructions, QFault is the first
> open-source C++ compiler pipeline that does this in a single unified tool.
> Unlike gluing Stim + GridSynth + PyLIQTR together, QFault provides a
> modular pass-manager architecture with spatial routing awareness and
> compiler-derived resource estimation.
