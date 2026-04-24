# QFault — Project Brief

**Type:** Open-source C++ library + arXiv technical report  
**License:** Apache 2.0  
**Timeline:** ~6–7 months solo / ~3–4 months two-person team  
**Target release:** v0.1.0 on GitHub + PyPI + arXiv preprint

---

## Problem Statement

Compiling a logical quantum circuit to fault-tolerant, physically executable
instructions for a surface code processor currently requires manually gluing
4–5 disconnected tools:

| Tool | Role | Gap |
|------|------|-----|
| Stim | Stabilizer simulation + decoding | Not a compiler; no circuit input |
| GridSynth / newsynth | Single-qubit T-gate synthesis | Standalone CLI; no pipeline |
| Qiskit Terra transpiler | Gate-level transpilation | Python-only; no FT-aware passes |
| PyLIQTR / Azure Quantum RE | Resource estimation | Heuristic; no actual scheduling |
| CUDA-Q / Intel SDK | MLIR-based quantum IR | Proprietary; vendor-locked |

**No unified, open-source, high-performance pipeline exists** that takes a
logical Clifford+T circuit and outputs a scheduled, spatially-routed,
fault-tolerant instruction stream in a single tool.

---

## Goals

1. **Primary:** Build a modular C++20 compiler-pass library (`libqfault`) with
   Python bindings that compiles logical quantum circuits to fault-tolerant
   surface-code execution sequences.

2. **Secondary:** Publish an arXiv technical report benchmarking the library
   against manual pipelines (Stim + GridSynth + PyLIQTR) on:
   - T-count after synthesis
   - Physical qubit footprint
   - Compilation time for Bernstein-Vazirani, small Shor instances

3. **Tertiary:** Establish community credibility and pursue an IEEE Quantum Week
   (QCE) or TQC follow-up paper post-v0.1.

---

## Non-Goals (explicit scope boundaries)

- **Not a simulator.** QFault does not simulate quantum circuits; Stim is used
  as an external oracle for validation.
- **Not hardware-connected.** No direct interface to physical quantum hardware;
  output is QASM 3.0 / QIR files.
- **Not a decoder.** Syndrome decoding (e.g., minimum-weight perfect matching)
  is an input dependency, not an output.
- **Not a full SDK.** No user-facing IDE, no cloud service, no circuit drawing.
- **Colour codes and other topological codes:** deferred post-v0.1. Surface
  code is the sole target for initial release.
- **Variable code distance per circuit region:** deferred. Global code distance
  `d` is a compiler parameter for v0.1.

---

## Success Criteria for v0.1.0

| Criterion | Measurement |
|-----------|-------------|
| Correct FT compilation | Stim oracle confirms logical output for BV + small Shor at d=5 |
| Competitive T-count | Within 10% of GridSynth standalone on single-qubit benchmarks |
| MSD scheduler beats naïve | ≥1 factory serves multiple T-gates; physical qubit count lower than "1 factory per T-gate" |
| ResourceEstimator tighter than PyLIQTR | Qubit-count estimates derived from actual routing decisions |
| Community signal | ≥1 substantive technical response within 4 weeks of arXiv post |
| Performance | Compiles 10k-gate BV circuit in <60s on a 16-core workstation |

---

## Stakeholders

- **Primary developer:** You (solo or small team)
- **Target users:** QEC researchers; quantum systems engineers at Google, IBM, Microsoft
- **Validators:** Stim (simulation oracle); published GridSynth T-count tables
- **Potential collaborators:** Academic QEC groups (Preskill group, Fowler, Campbell)
