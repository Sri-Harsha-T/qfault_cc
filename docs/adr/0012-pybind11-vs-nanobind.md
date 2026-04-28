# ADR-0012: Python bindings — pybind11 for v0.1, revisit nanobind for v0.2

**Status:** Accepted
**Date:** 2026-04-26
**Supersedes:** —

---

## Context

`CMakeLists.txt` already declares pybind11 v2.11.1 as an optional
FetchContent dependency for Stage 5b. nanobind is the modern alternative
(roughly 4× faster compile, ~5× smaller binary, Stable-ABI single-wheel
via cibuildwheel). The team must decide whether to switch now (before
Stage 5b ramp-up) or defer.

---

## Decision

**v0.1 uses pybind11 v2.11.1** as already configured in `CMakeLists.txt`,
to preserve Stage 5b velocity. **v0.2 will revisit nanobind** with a
measured comparison on the QFault binding surface (compile time, wheel
size, build matrix coverage).

Switching now would lose 2–3 weeks of Stage 5b time without a measured
benefit on QFault's specific binding surface. The decision is sequenced,
not foreclosed.

---

## Alternatives Considered

| Alternative | Why Rejected |
|---|---|
| Switch to nanobind in v0.1 | Loses 2–3 weeks of Stage 5b time; benefit not yet measured against QFault's binding surface |
| Boost.Python | Over-engineered, slower compile, larger binary |
| cppyy / clif | Smaller communities; less reviewer recognition |
| No Python bindings in v0.1 | Cuts against Stage 5b ROI; the QEC research community is Python-first |

---

## Consequences

**Positive:**
- Already-configured dependency; no additional Stage 5b ramp-up.
- cibuildwheel works with pybind11 across Linux, macOS, Windows.
- Wide community familiarity for Stage 5c arXiv reviewers.

**Negative / Trade-offs:**
- Larger wheel size and slower compile vs nanobind; acceptable for v0.1.
- Stable ABI requires extra discipline; mitigated by limiting the binding
  surface to a small core (`compile`, `parse`, `ResourceReport`,
  `PassContext`).

**Risks:**
- v0.2 nanobind switch may require minor API adjustments at the binding
  layer; documented as a known v0.2 task in the roadmap.
- pybind11 v2.11.1 + Python 3.13 compatibility: monitor upstream;
  `cibuildwheel` action should pin Python versions to 3.10–3.12 if 3.13
  issues appear.

---

## Implementation Notes for AI Sessions

When loading the memory bank:
- pybind11 v2.11.1 in `CMakeLists.txt` for v0.1 (already configured).
- Do NOT switch to nanobind without an updated ADR with **measured**
  numbers on the QFault binding surface.
- Binding source lives in `bindings/python/` (Stage 5b deliverable).
- `pyproject.toml` uses `scikit-build-core` for the build backend.
- Package name on PyPI: `qfault`.

---

## References

- pybind11 documentation: https://pybind11.readthedocs.io/
- nanobind benchmarks (Wenzel Jakob):
  https://github.com/wjakob/nanobind/blob/master/docs/benchmark.rst
- cibuildwheel: https://cibuildwheel.pypa.io/
- ADR-0011 (PyZX bridge — uses the same pybind11 boundary)
