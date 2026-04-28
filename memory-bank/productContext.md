# Product Context — QFault

## Why This Project Exists

The quantum computing industry is entering the "utility era" where fault-tolerant
demonstrations are actively being pursued by Google (Willow), IBM (Heron), and
Microsoft (Majorana 1). The hardware bottleneck is shifting: **the new bottleneck
is compilation infrastructure**.

Google's open quantum compiling roles explicitly list "compiling logical quantum
algorithms to low-level fault-tolerant instructions" as the core task. This is
a funded, active need — and the open-source tooling to support it is fragmentary.

## Problem statement

Compiling a logical quantum algorithm to fault-tolerant instructions on a
surface code requires solving five sub-problems in sequence: (1) decomposing
arbitrary single-qubit rotations into Clifford+T sequences, (2) routing
multi-qubit operations as lattice surgeries on a 2D tile grid, (3) scheduling
operations against limited factory outputs and routing channels, (4) sizing
the magic-state-distillation factories to meet T-state demand at the target
output error, and (5) estimating physical-qubit count and wall-clock time.

Each sub-problem has open-source tools (GridSynth, liblsqecc, Stim, QFold,
Qiskit-QEC, Azure QRE), but they are written in five different languages
(Haskell, C++, Python, F#, C#) with five different IRs. There is no single
codebase that takes a circuit in and emits a complete fault-tolerant
specification with intermediate artifacts that other tools accept.

## How QFault solves it

A single C++20 codebase, structured as **LLVM-style passes** over a **two-level
IR** (`std::variant<LogicalGate, PatchOp>`), with **C++20 Concept-based
provider interfaces** at extension points. The FT-specific knowledge is
captured in **ADRs** (architectural decisions) and **path-scoped rules** so
that the codebase explains itself to maintainers and reviewers.

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

## What QFault explicitly does not do

- Execute on hardware (we emit; we don't dispatch)
- Decode (we emit Stim circuits; the user runs Stim/PyMatching/etc.)
- Compile from a high-level language (we accept QASM 3.0; not Q#, not Python)
- Optimise across the Pauli-product / phase-polynomial frontier in v0.1
  (Stage 3 or 6 deliverable, ADR-0011)

## User journey (v0.1)

```
$ cat my_algorithm.qasm
OPENQASM 3.0;
include "stdgates.inc";
qubit[10] q;
... 10⁶ T-gates total ...

$ qfault compile my_algorithm.qasm \
    --code-distance 13 --error-rate 1e-4 \
    --backend qir --emit-stim-oracle out/

Logical:    10 qubits, 1.2e6 T-count
Synthesis:  GridSynth ε=1e-10, 8.4 sec
Routing:    intermediate layout, 24 tiles, 5τ/T
MSD:        15-to-1 SE × 16, output P_T = 5.6e-11
Estimate:   53,200 physical qubits, 4.1 hours @ 1µs cycle
Backend:    out/algorithm.ll  (QIR, 12 KB)
Oracle:     out/algorithm.stim (Stim circuit, 2.1 MB)

$ qfault verify out/algorithm.stim my_algorithm.qasm
[Stim has_flow check]   PASS (10/10 logical observables)
[QCEC clifford+T check] PASS (all 7-qubit Clifford+T blocks)
[golden detector check] PASS (1024/1024 noiseless shots, 0 detector flips)
```

This is the experience v0.1 ships.