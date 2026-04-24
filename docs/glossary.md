# QFault Glossary

> Load this file when working on QEC-specific code or when a domain term
> is ambiguous. Terms are ordered from most-fundamental to most-specialised.

---

## Quantum Error Correction Fundamentals

**Qubit** — A two-state quantum system. Basis states |0⟩ and |1⟩.

**Pauli operators** — The four 2×2 matrices: I (identity), X (bit-flip),
Z (phase-flip), Y = iXZ. Errors are decomposed into Pauli errors.

**Clifford gates** — Gates that map Pauli operators to Pauli operators under
conjugation: H (Hadamard), S (phase), CNOT, CZ, Pauli X/Y/Z. Efficiently
simulable classically (Gottesman-Knill theorem).

**T-gate** — The π/8 rotation gate; NOT in the Clifford group. Required for
universal quantum computation. Expensive to implement fault-tolerantly.

**Clifford+T gate set** — Universal gate set: {H, S, CNOT, T}. Any unitary
can be approximated to arbitrary precision using these gates.

**Stabilizer code** — A quantum error correcting code defined by a group of
commuting Pauli operators (the stabilizers). Codewords are the simultaneous
+1 eigenstates of all stabilizers.

**Code distance d** — Minimum weight of a Pauli error that is logically
undetectable. Higher d → better error protection → more physical qubits.
In QFault v0.1, d is a global compiler parameter.

**Syndrome** — The measurement outcome of the stabilizer generators. Tells
the decoder where errors occurred, not what the logical state is.

**Syndrome extraction** — The circuit that measures all stabilizer generators
in one "round." Depth ≤ 6 layers for surface code.

**Logical qubit** — A single qubit encoded in many physical qubits via an
error-correcting code. In the surface code, one logical qubit needs d²
physical data qubits + (d²-1) measurement qubits.

---

## Surface Code

**Surface code** — A 2D topological stabilizer code on a square lattice of
physical qubits. The most hardware-relevant QEC code for near-term devices.
(Also: toric code, rotated surface code — QFault targets rotated surface code.)

**Data qubit** — Physical qubit that stores the encoded quantum information.

**Measure qubit / ancilla qubit** — Physical qubit used to measure stabilizers.
Not part of the logical state.

**X-stabilizer / Z-stabilizer** — The two types of plaquette checks in the
surface code. X-stabilizers detect Z errors; Z-stabilizers detect X errors.

**Logical X / Logical Z** — The logical observables of the encoded qubit.
Logical X is a string of X operators along one edge; Logical Z is a string
of Z operators along the perpendicular edge.

**Code patch / Patch** — A rectangular region of the surface code grid that
encodes one logical qubit. In QFault, a patch is `PatchCoord{x, y}` plus
its orientation (X-boundary left/right or Z-boundary top/bottom).

**Patch orientation** — Which boundaries are X-type and which are Z-type.
Orientation determines how logical gates are implemented.

---

## Lattice Surgery

**Lattice surgery** — A method for implementing logical gates between surface
code patches without physically moving qubits. Uses merge and split operations.

**Merge** — Temporarily merge two patches by measuring their shared boundary
stabilizers. Implements a logical two-qubit Pauli measurement.

**Split** — Separate a merged patch back into two by ceasing boundary measurements.

**Merge-Split sequence** — The primitive for logical CNOT:
`MERGE(control_X, target_X) → measure → SPLIT`. Takes one surface code cycle.

**Ancilla patch / Bus patch** — Intermediate patches used to bridge non-adjacent
data patches during routing.

**Routing** — The process of placing bus patches between non-adjacent logical
qubits so they can interact via lattice surgery. O(Manhattan distance) ancilla overhead.

**Lattice surgery time step** — One round of syndrome extraction + classical
decoding + correction. Typically ~1μs on superconducting hardware.

---

## Magic State Distillation (MSD)

**Magic state** — The state |T⟩ = T|+⟩ = (|0⟩ + e^{iπ/4}|1⟩)/√2. Consuming
a magic state via a Clifford circuit + teleportation implements a logical T-gate.

**Magic state distillation** — A protocol that takes many noisy magic states
and outputs fewer, higher-fidelity magic states. Most common: 15-to-1 protocol
(consumes 15 noisy |T⟩ states to produce 1 high-fidelity one).

**MSD factory** — A dedicated region of the patch grid that continuously runs
a distillation protocol. Produces one magic state every `cycleTime` steps.

**T-gate factory** / **CCZ factory** — Specific factory designs for T-gate or
CCZ-gate magic state production. Different physical footprints.

**Factory footprint** — The number of physical qubits occupied by an MSD factory.
A typical 15-to-1 factory at code distance d occupies ~100–1000 qubits.

**Factory throughput** — Magic states per unit time. More factories → more
throughput → shorter overall circuit time, at the cost of more physical qubits.

---

## Synthesis

**Gate synthesis** — The process of decomposing an arbitrary single-qubit
unitary into a sequence of Clifford+T gates.

**Solovay-Kitaev (SK) algorithm** — A recursive algorithm for gate synthesis.
Produces gate sequences of length O(log^c(1/ε)) for approximation error ε.
Classic but not T-count optimal. Used in QFault as a benchmark baseline only.

**GridSynth** — A number-theoretic synthesis algorithm by Ross and Selinger.
Produces nearly T-count-optimal decompositions. State of the art for
single-qubit synthesis. Used in QFault as the default SynthesisProvider.

**T-count** — The number of T-gates in a synthesised gate sequence. Lower is
better — each T-gate requires one magic state and one MSD factory cycle.

**T-depth** — The number of layers of T-gates (counting parallel T-gates as
depth 1). Determines the minimum circuit time.

**Approximation error ε** — The diamond-norm distance between the target
unitary and the synthesised sequence. Synthesis produces sequences with ε < given threshold.

---

## Compiler Terms (QFault-specific)

**QFaultIR** — QFault's internal representation. Two levels: logical (Clifford+T
gates over logical qubits) and physical (patch ops over 2D patch coordinates).

**IRLevel** — Enum tag on `QFaultIRModule`: `LOGICAL` or `PHYSICAL`.

**PassManager** — The LLVM-inspired component that chains compiler passes
over `QFaultIRModule`.

**SynthesisProvider** — A C++20 Concept defining the interface for gate
synthesis algorithms. Current implementors: `GridSynthProvider`, `SKProvider`.

**PassContext** — Context object threaded through a pass run: holds configuration
(code distance, synthesis choice), diagnostics, and timing.

**ResourceReport** — Output of `ResourceEstimatorPass`. Contains:
`physicalQubits`, `timeSteps`, `codeDistance`, `msdFactoryCount`, `tGateCount`.

**Lowering pass** — The pass that converts IR from `LOGICAL` level to
`PHYSICAL` level. Implemented as `LatticeSurgeryPass`.

**Stim oracle** — The external Stim simulator used to validate that QFault's
compiled output produces the correct logical measurement outcomes.
