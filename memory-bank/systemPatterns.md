# System Patterns — QFault

## Top-Level Architecture

```
[Input: Logical Circuit (Clifford+T)]
        │
        ▼
┌───────────────────────────────┐
│        Frontend Parser         │  QASM 3.0 / Cirq JSON / custom IR
└───────────────┬───────────────┘
                │ QFaultIR (logical)
                ▼
┌───────────────────────────────┐
│         PassManager            │  Chains compiler passes
│  ┌────────────────────────┐   │
│  │  SynthesisPass          │◄──┤  GridSynth (default) or SK (benchmark)
│  │  (T-gate decomposition) │   │
│  └────────────────────────┘   │
│  ┌────────────────────────┐   │
│  │  LatticeSurgeryPass     │◄──┤  Logical → patch merge/split sequences
│  │  (CNOT → patch ops)     │   │
│  └────────────────────────┘   │
│  ┌────────────────────────┐   │
│  │  MSDSchedulerPass       │◄──┤  Factory placement + routing delays
│  │  (T-gate scheduling)    │   │
│  └────────────────────────┘   │
│  ┌────────────────────────┐   │
│  │  ResourceEstimatorPass  │   │  Physical qubits, time-steps, code distance
│  └────────────────────────┘   │
└───────────────┬───────────────┘
                │ QFaultIR (physical / scheduled)
                ▼
┌───────────────────────────────┐
│      Output Backend            │  QASM 3.0 emitter OR QIR emitter
└───────────────────────────────┘
```

## Core Design Patterns

### 1. PassManager Pattern (LLVM-inspired)
- Passes are composable, ordered transformations over `QFaultIR`
- Each pass implements `virtual void run(QFaultIRModule&, PassContext&)`
- Passes declare read/write requirements for dependency ordering
- Pattern: `PassManager pm; pm.add<SynthesisPass>(); pm.run(module);`

### 2. SynthesisProvider Interface (Strategy Pattern via Concepts)
```cpp
// Concept — not a virtual base class
template<typename T>
concept SynthesisProvider = requires(T s, UnitaryMatrix u) {
    { s.synthesize(u) } -> std::same_as<CliffordTPlusList>;
    { s.tCount(u) } -> std::same_as<std::size_t>;
};
```
- GridSynth: wraps GridSynth CLI/lib — primary provider, T-optimal
- SolovayKitaev: implemented in-library — benchmark/baseline only
- New providers can be dropped in without modifying `SynthesisPass`

### 3. Two-Level IR
- **Logical IR** (`QFaultIR::LogicalGate`): Clifford+T gates over logical qubits
- **Physical IR** (`QFaultIR::PatchOp`): surface code patch merge/split/measure ops
- A `Lowering` pass converts logical → physical; they share the same `QFaultIRModule`
  via a discriminated union, NOT two separate IR types
- **Stage 1 gate question:** can one module cleanly hold both levels simultaneously?
  If not, two IRs with a lowering pass (doubles design complexity — avoid if possible)

### 4. Spatial Patch Representation
- Surface code patches are 2D grid cells — represented as `PatchCoord{x, y}`
- MSD factories are reserved rectangular regions: `FactoryRegion{origin, width, height}`
- Routing is Manhattan distance on the patch grid (greedy heuristic for v0.1)
- Routing complexity is NP-hard in general; heuristic must document approximation bounds

### 5. Resource Estimation as a Compiler Pass
- `ResourceEstimatorPass` runs after `MSDSchedulerPass`
- Derives estimates from actual routing decisions, not heuristic formulas
- Output: `ResourceReport{physicalQubits, timeSteps, codeDistance, msdFactoryCount}`

### 6. Output Backends as Visitors
- Both QASM 3.0 and QIR emitters implement a Visitor over `QFaultIR::PatchOp`
- Swapping backends does not change any pass internals
- QIR spec pinned to a specific version in `CMakeLists.txt`

## Key Architectural Constraints

| Constraint | Rationale |
|-----------|-----------|
| C++20, not Python | Shor's at scale needs millions of T-gates; Python is not viable |
| Concepts over virtual | Avoids vtable overhead in hot loops; cleaner template errors |
| No `std::variant` in hot paths | Compile-time blowup with heavy template instantiation (learned) |
| Global code distance `d` for v0.1 | Variable-d is theoretically correct but dramatically complicates routing |
| Greedy routing heuristic | NP-hard exact routing is not feasible; must document bounds clearly |
| GridSynth as default, SK as benchmark | SK is deprecated for T-count optimality; must not be the default |
| Stim as oracle, not replaced | No physical hardware; Stim validates correct logical output |

## Testing Architecture

```
tests/
  unit/          # Per-class unit tests (GoogleTest)
  integration/   # End-to-end: circuit → QIR → Stim round-trip
  benchmarks/    # Google Benchmark: T-count, qubit footprint, compile time
  reference/     # Golden outputs from Stim oracle for regression
```

- Round-trip test pattern: `encode circuit → compile via QFault → simulate via Stim → compare`
- Reference outputs committed at `tests/reference/*.stim` — diff on CI
- Benchmarks are not part of the regular test run (gated by `QFAULT_RUN_BENCHMARKS=ON`)
