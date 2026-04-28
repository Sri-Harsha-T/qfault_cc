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

### 1. Two-Level IR with `std::variant` (ADR-0001)
- **Logical IR** (`QFaultIR::LogicalGate`): Clifford+T gates over logical qubits
- **Physical IR** (`QFaultIR::PatchOp`): surface code patch merge/split/measure ops
- A `Lowering` pass converts logical → physical; they share the same `QFaultIRModule`
  via a discriminated union, NOT two separate IR types
```cpp
using Instruction = std::variant<LogicalGate, PatchOp>;

class QFaultIRModule {
    IRLevel level_;
    std::vector<Instruction> instrs_;
public:
    void assertLevel(IRLevel required) const {
        if (level_ != required) throw std::logic_error(...);
    }
    // ...
};
```

Every pass starts: `module.assertLevel(requiredLevel());`

### 2. SynthesisProvider Interface (Strategy Pattern via Concepts)
```cpp
// Concept — not a virtual base class
template<typename T>
concept SynthesisProvider = requires(T s, UnitaryMatrix u) {
    { s.synthesize(u) } -> std::same_as<CliffordTPlusList>;
    { s.tCount(u) } -> std::same_as<std::size_t>;
};
```
```cpp
template <typename T>
concept SynthesisProvider = requires(T p, double a, double e) {
    { p.synthesise(a, e) } -> std::convertible_to<std::vector<GateKind>>;
    { p.name() }           -> std::convertible_to<std::string_view>;
};

template <SynthesisProvider P>
class TGateSynthesisPass : public PassBase {
    P provider_;
public:
    PassResult run(QFaultIRModule& m, PassContext& ctx) override { ... }
};
```
Provider swap is a one-line template-argument change. Zero overhead at the
Concept boundary, confirmed by `static_assert` tests.
- GridSynth: wraps GridSynth CLI/lib — primary provider, T-optimal
- SolovayKitaev: implemented in-library — benchmark/baseline only
- New providers can be dropped in without modifying `SynthesisPass`

### 3. PassManager Pattern (LLVM-inspired) with telemetry
- Passes are composable, ordered transformations over `QFaultIR`
- Each pass implements `virtual void run(QFaultIRModule&, PassContext&)`
- Passes declare read/write requirements for dependency ordering
- Pattern: `PassManager pm; pm.add<SynthesisPass>(); pm.run(module);`
```cpp
class PassBase {
public:
    virtual ~PassBase() = default;
    virtual PassResult run(QFaultIRModule& m, PassContext& ctx) = 0;
    virtual std::string_view name() const = 0;
    virtual IRLevel requiredLevel() const = 0;
};

class PassManager {
    std::vector<std::unique_ptr<PassBase>> passes_;
    std::vector<PassStats> stats_;
public:
    template <std::derived_from<PassBase> P, class... Args>
    PassManager& add(Args&&... args);  // perfect-forward construct, push_back
    PassResult run(QFaultIRModule& m, PassContext& ctx);
    void printStats(std::ostream& os) const;  // fixed-width columns
};
```

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

### Diagnostics via PassContext, never std::cerr

```cpp
class PassContext {
public:
    void addDiagnostic(DiagLevel lvl, std::string msg);
    [[nodiscard]] int codeDistance() const noexcept { return d_; }
    [[nodiscard]] double synthesisEpsilon() const noexcept { return eps_; }
    // ...
};
```

The pass writes to `ctx.addDiagnostic(DiagLevel::Warn, "...")`. The CLI driver
or test fixture decides what to do with the diagnostics.

### Result type via tl::expected (Stage 3+)

```cpp
template <class T>
using Result = tl::expected<T, QFaultError>;

Result<RoutedCircuit> route(const PatchSpec& spec) {
    if (!spec.is_valid()) return tl::unexpected{QFaultError::InvalidSpec};
    // ...
}

route(spec)
    .and_then(schedule)
    .and_then(emit_stim)
    .or_else(report_error);
```

`tl::expected` is the C++20 shim for `std::expected`; trivial migration when
the toolchain reaches C++23.

### Sanitizer-clean A* graph algorithm (ADR-0006)

- `using NodeId = std::uint32_t;` — never `Node*`
- Heap key: `(f_score, neg_g_score, insertion_seq)` tuple — deterministic tie-break
- Costs: signed `int64_t` so UBSAN catches overflow
- Closed list: dense `std::vector<uint8_t>`, not `std::unordered_set`
- Lazy deletion (`if (closed[cur.node]) continue;` after pop)

### `std::visit` with `overload{}` helper

```cpp
template <class... Ts> struct overload : Ts... { using Ts::operator()...; };
template <class... Ts> overload(Ts...) -> overload<Ts...>;

std::visit(overload{
    [&](const LogicalGate& g) { ... },
    [&](const PatchOp& op)    { ... },
}, instr);
```

### Failed-approach log as living document

`CHANGELOG.md` "Failed Approaches" is **append-only**. Every dead end gets:
date, approach, why-failed, replacement. The `/log` slash command appends.
The `reviewer` agent reads this before evaluating any change.

### Memory-bank as session continuity

`memory-bank/{activeContext, productContext, progress, projectbrief, systemPatterns,
techContext}.md` carries state across sessions. `activeContext.md` is updated
every session before `/clear`; the others are stable across many sessions.

### Stage gates as falsifiable predicates

Every stage has an exit predicate of the form *"On reference circuit X, does
QFault produce result Y within Z% of published number?"* The predicate is
encoded in `tests/integration/test_stage_N_gate.cpp`. CI runs it on every PR.

### Path-scoped rules (`.claude/rules/*.md`)

Each rules file declares the path glob it applies to in YAML front-matter.
Claude Code loads only the relevant rules for each file under edit, keeping
the active rule budget small. `cpp.md` (all C++), `qec.md` (passes/IR/verify),
`routing.md` (Stage 3 routing).

### Specialised subagents (`.claude/agents/*.md`)

`cpp-pro` for sanitizer-tricky modern C++ work. `reviewer` for fresh-context PR
review. Activated via the Task tool with appropriate subagent_type.

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
