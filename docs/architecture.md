# QFault Architecture

> **When to read this:** When implementing a new compiler pass, when changing
> the IR schema, or when evaluating a new dependency. For quick orientation
> per-session, use `memory-bank/systemPatterns.md` instead.

---

## Overview

QFault is structured as an **LLVM-inspired pass manager** over a two-level
internal IR (`QFaultIR`). The pipeline is:

```
QASM 3.0 / Cirq JSON
    │
    ▼  [Frontend Parser]
QFaultIR (logical level)
    │
    ▼  [SynthesisPass]         — decomposes arbitrary gates to Clifford+T
QFaultIR (logical, Clifford+T only)
    │
    ▼  [LatticeSurgeryPass]    — logical → patch merge/split/measure ops
QFaultIR (physical level)
    │
    ▼  [MSDSchedulerPass]      — schedules T-gates via MSD factories
QFaultIR (physical + scheduled)
    │
    ▼  [ResourceEstimatorPass] — derives qubit/time-step counts
ResourceReport
    │
    ▼  [Output Backend]        — QASM 3.0 or QIR emitter
File output
```

---

## QFaultIR Design

### The Two-Level Problem

A key design decision (see ADR-0001) is how to represent both:
- **Logical gates:** `H`, `CNOT`, `T`, `S` over named logical qubits
- **Physical patch ops:** `MERGE(p1, p2)`, `SPLIT(p)`, `MEASURE(p, basis)`
  over 2D patch coordinates

**Chosen approach:** A single `QFaultIRModule` containing an `std::vector<Instruction>`
where `Instruction` is a `std::variant<LogicalGate, PatchOp>`. The module
tracks its own "lowering level" as a tag — passes assert the correct level
before operating.

**Rejected approaches:** See CHANGELOG.md "Failed Approaches".

### Key Data Structures

```cpp
// Logical level
struct LogicalQubit { std::string name; std::size_t index; };
struct LogicalGate  { GateKind kind; std::vector<LogicalQubit> operands;
                      std::optional<double> angle; };

// Physical level
struct PatchCoord   { int x, y; };
struct PatchOp      { PatchOpKind kind;           // MERGE, SPLIT, MEASURE, IDLE
                      std::vector<PatchCoord> patches;
                      MeasBasis basis;             // X, Z, Y
                      TimeStep t; };

// Module
struct QFaultIRModule {
    std::string name;
    IRLevel level;        // LOGICAL or PHYSICAL
    std::vector<LogicalQubit> qubits;
    std::vector<Instruction> instructions;  // variant<LogicalGate, PatchOp>
};
```

---

## Pass Manager

```cpp
class PassManager {
public:
    template<typename PassT, typename... Args>
    PassManager& add(Args&&... args);

    PassResult run(QFaultIRModule& module);

private:
    std::vector<std::unique_ptr<PassBase>> passes_;
};

class PassBase {
public:
    virtual ~PassBase() = default;
    virtual std::string_view name() const = 0;
    virtual IRLevel requiredLevel() const = 0;
    virtual PassResult run(QFaultIRModule&, PassContext&) = 0;
};
```

`PassContext` provides: logging, diagnostic collection, timing, and access to
pass-specific configuration (e.g., code distance `d`, synthesis algorithm choice).

---

## Lattice Surgery

Lattice surgery realises logical gates by merging and splitting surface code
patches according to the following primitive operations:

| Logical Gate | Lattice Surgery Sequence |
|--------------|-------------------------|
| Logical CNOT (data→anc) | MERGE(data_X, anc_X) → MEASURE(anc_X, X) → SPLIT |
| Logical H | Change patch orientation (X↔Z boundary swap) |
| Logical S | Inject Y eigenstate; consume via MERGE |
| Logical T | Consume magic state from MSD factory via CNOT-like merge |

**Routing:** Patches that are non-adjacent must have a "bus" of ancilla patches
bridging them. QFault v0.1 uses a greedy Manhattan-distance routing heuristic.
The routing graph is a 2D grid; the heuristic is O(n²) in patch count.

**Documented limitation:** Greedy routing can produce infeasible layouts at
~10,000+ logical qubits. A backtracking solver is a v0.2 item.

---

## Magic State Distillation (MSD) Scheduling

Magic states (|T⟩ = T|+⟩) are consumed one per logical T-gate. They are
produced by **distillation factories** — fixed regions of the patch grid that
run a distillation protocol (e.g., 15-to-1 or CCZ factory).

**Factory model in QFault:**
- A factory is a `FactoryRegion{PatchCoord origin, int width, int height, int cycleTime}`
- `cycleTime` is how many time steps to produce one magic state
- `MSDSchedulerPass` assigns T-gate requests to factory outputs, accounting for
  routing delay from factory location to data qubit location

**Scheduling algorithm (v0.1):** Earliest-available factory first.
Known limitation: this can be suboptimal for bursty T-gate sequences.

---

## Resource Estimation

`ResourceEstimatorPass` produces a `ResourceReport` by aggregating:

```
physicalQubits  = dataPatches * d² + ancillaPatches * d²
                  + sum(factoryFootprints * d²)
timeSteps       = max_t over all patch operations (critical path)
codeDistance    = d (global compiler parameter)
msdFactoryCount = number of distinct factory regions placed
tGateCount      = total T-gates in the logical input circuit
```

Output methods: `report.toJSON()`, `report.toMarkdown()`, `report.toLatex()`.

---

## Output Backends

### QASM 3.0 Backend
Emits standard OpenQASM 3.0 gate instructions. Physical patch ops are
emitted as custom gate calls into a QFault-specific standard library
(`include "qfault_stdlib.qasm"`).

### QIR Backend (Stage 5)
Emits QIR v1.0 (pinned — see ADR-0005). Uses direct string emission,
not LLVM IR, for v0.1. MLIR integration is a v0.2 item.

Both backends implement:
```cpp
class BackendBase {
public:
    virtual void emit(const QFaultIRModule&, std::ostream&) = 0;
};
```
