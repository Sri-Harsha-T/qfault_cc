# QFault Roadmap

## Timeline Overview

```
Month 1        Month 2        Month 3        Month 4        Month 5-6      Month 6-7
│              │              │              │              │              │
▼              ▼              ▼              ▼              ▼              ▼
┌──────────┐  ┌───────────┐  ┌───────────┐  ┌───────────┐  ┌──────────┐  ┌──────────┐
│ Stage 1  │  │  Stage 2  │  │  Stage 3  │  │  Stage 4  │  │ Stage 5  │  │  arXiv   │
│  IR +    │  │ Synthesis │  │  Lattice  │  │   MSD +   │  │ Backends │  │ Preprint │
│  PassMgr │  │   Pass    │  │  Surgery  │  │  ResEst   │  │ + Python │  │ + v0.1.0 │
└──────────┘  └───────────┘  └───────────┘  └───────────┘  └──────────┘  └──────────┘
   3-4 wks       4-6 wks        6-8 wks        6-8 wks        4-5 wks
   Gate: IR      Gate: T-cnt    Gate: Stim      Gate: MSD      Gate: community
   holds both    ≤5% overhead   oracle passes   beats naïve    signal in 4 wks
   levels clean  vs GridSynth   at d=5          alloc
```

---

## Stage 1: IR + Pass Manager Core (Weeks 1–4)

**Goal:** The structural foundation. No QEC logic yet.

| Deliverable | Description |
|-------------|-------------|
| `QFaultIRModule` | Two-level IR (logical + physical) in one module |
| `PassManager` | Composable, ordered pass runner with diagnostics |
| `NoOpPass` | Validates the framework end-to-end |
| QASM 3.0 parser (subset) | Reads Clifford+T circuits |
| CMake build + CI | clang + gcc matrix, ASAN, ctest |

**Stage Gate:**
> The IR cleanly represents both logical Clifford+T AND surface code patch ops
> in one data structure without lossy conversion. NoOpPass round-trip test passes.

**GitHub Milestone:** `Stage 1: IR + Pass Manager Core`

---

## Stage 2: Synthesis Pass — T-Gate Decomposition (Weeks 4–9)

**Goal:** Decompose arbitrary single-qubit unitaries into Clifford+T sequences.

| Deliverable | Description |
|-------------|-------------|
| `SynthesisProvider` Concept | Pluggable interface (ADR-0002) |
| `GridSynthProvider` | Wraps GridSynth binary/lib — primary, T-optimal |
| `SKProvider` | Pure C++ Solovay-Kitaev — benchmark baseline only |
| `SynthesisPass` | Applies provider to all non-Clifford gates in module |
| T-count benchmarks | Validated against published GridSynth tables |

**Stage Gate:**
> `SynthesisProvider` abstraction adds ≤5% runtime overhead vs calling GridSynth
> directly on a 1,000-gate circuit.

**GitHub Milestone:** `Stage 2: Synthesis Pass`

---

## Stage 3: Lattice Surgery Mapper (Weeks 9–16)

**Goal:** Translate logical Clifford+T into surface code patch operations.

| Deliverable | Description |
|-------------|-------------|
| `LatticeSurgeryPass` | Converts logical CNOT to MERGE/SPLIT/MEASURE sequences |
| Patch grid model | 2D `PatchCoord` grid with ancilla routing |
| Greedy routing heuristic | Manhattan-distance routing; documents approximation bounds |
| Stim oracle integration | `scripts/compare-stim.sh` validates compiled output |
| Integration tests | 10-qubit Bernstein-Vazirani at code distance d=5 |

**Stage Gate:**
> For a 10-logical-qubit Bernstein-Vazirani circuit, greedy lattice surgery routing
> produces a patch schedule that Stim simulates with correct logical output at d=5.

**GitHub Milestone:** `Stage 3: Lattice Surgery Mapper`

---

## Stage 4: MSD Scheduling + Resource Estimator (Weeks 16–23)

**Goal:** Schedule T-gate consumption via MSD factories; estimate real resource costs.

| Deliverable | Description |
|-------------|-------------|
| MSD factory spatial model | `FactoryRegion{origin, width, height, cycleTime}` |
| `MSDSchedulerPass` | Earliest-available factory assignment with routing delays |
| `ResourceEstimatorPass` | Derives physical qubits + time-steps from actual compilation |
| `ResourceReport` | JSON + Markdown + LaTeX output formats |
| Benchmark vs PyLIQTR | Qubit estimates on BV + small Shor instance |

**Stage Gate:**
> MSD factory scheduler produces a lower physical qubit count than the naïve
> "one factory per T-gate" baseline on a circuit with ≥50 T-gates.

**GitHub Milestone:** `Stage 4: MSD Scheduling + Resource Estimator`

---

## Stage 5: Output Backends + Python Bindings + arXiv (Weeks 23–28)

**Goal:** Complete the pipeline, expose Python API, publish.

| Deliverable | Description |
|-------------|-------------|
| QASM 3.0 output backend | Emits standard QASM 3.0 + QFault stdlib |
| QIR output backend | QIR v1.0 (pinned spec — ADR-0005) |
| pybind11 Python bindings | Full pipeline accessible from Python ≥3.10 |
| PyPI package | `pip install qfault` |
| arXiv technical report | Architecture, benchmarks, related work, limitations |
| v0.1.0 GitHub release | Tagged release with changelog |

**Stage Gate:**
> arXiv preprint receives ≥1 substantive technical response from QEC research
> community within 4 weeks of posting.

**GitHub Milestone:** `Stage 5: Output + Release + arXiv`

---

## Post-v0.1 Roadmap (v0.2+)

| Item | Priority | Rationale |
|------|----------|-----------|
| Variable code distance per circuit region | High | Reduces physical qubit overhead significantly |
| MLIR / QIR via LLVM proper | Medium | Future-proof output; required for hardware integration |
| Colour code support | Medium | Broadens applicability beyond surface code |
| Backtracking router for large circuits | Low | Required for >10k logical qubits |
| Formal equivalence checker | Research | Needed to claim fault-tolerant *correctness*, not just structure |
| GridSynth-ε variants | Medium | Further T-count reduction |
| Conference submission (IEEE QCE or TQC) | High | Peer-reviewed credibility |

---

## Key Risk Mitigations

| Risk | Mitigation |
|------|-----------|
| Lattice surgery routing NP-hard | Greedy heuristic v0.1; document bounds; backtracker deferred |
| SK deprecated for T-count | GridSynth is default; SK is benchmark only (ADR-0004) |
| QIR spec instability | Pin to specific QIR version in ADR-0005; modular backend |
| No real hardware to test | Stim as simulation oracle throughout |
| pybind11 maintenance debt | Keep binding surface minimal; public C API first |
| Context rot over 6 months | memory-bank system + CHANGELOG failed approaches log |
