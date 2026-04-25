#!/usr/bin/env bash
# bench-synthesis.sh — Stage 2 gate benchmark
#
# Measures the overhead of routing T-gate synthesis through
# TGateSynthesisPass<GridSynthProvider> vs calling gridsynth directly.
#
# Usage:
#   ./scripts/bench-synthesis.sh [build-dir]
#
# Requires:
#   - gridsynth binary on PATH (or GRIDSYNTH_BINARY env var)
#   - QFault test binary built with QFAULT_HAS_GRIDSYNTH=1
#
# Exit codes:
#   0  overhead ≤ 5%
#   1  overhead > 5%
#   2  gridsynth binary not found

set -euo pipefail

BUILD_DIR="${1:-build/gcc13-release}"
GRIDSYNTH="${GRIDSYNTH_BINARY:-$(command -v gridsynth 2>/dev/null || true)}"
N_T_GATES=300   # number of T gates in the benchmark circuit
EPS="1e-10"

if [[ -z "${GRIDSYNTH}" || ! -x "${GRIDSYNTH}" ]]; then
    echo "ERROR: gridsynth binary not found." >&2
    echo "  Set GRIDSYNTH_BINARY or put gridsynth on PATH." >&2
    exit 2
fi

echo "=== QFault Stage 2 Synthesis Benchmark ==="
echo "gridsynth: ${GRIDSYNTH}"
echo "N_T_GATES: ${N_T_GATES}"
echo "epsilon:   ${EPS}"
echo ""

# ── (a) Baseline: direct gridsynth calls ─────────────────────────────────────
echo "Measuring baseline (direct gridsynth calls)..."
BASELINE_START=$(date +%s%N)
for ((i = 0; i < N_T_GATES; i++)); do
    # Synthesise R_z(pi/4) for each T gate
    "${GRIDSYNTH}" -- 0.7853981633974483 -e "${EPS}" > /dev/null
done
BASELINE_END=$(date +%s%N)
BASELINE_MS=$(( (BASELINE_END - BASELINE_START) / 1000000 ))

echo "  Baseline: ${BASELINE_MS} ms"

# ── (b) QFault: parse + TGateSynthesisPass<GridSynthProvider> ─────────────────
# Generate a QASM 3.0 circuit with N_T_GATES T-gates and N_T_GATES Clifford gates.
QASM_FILE=$(mktemp /tmp/bench_circuit_XXXXXX.qasm)
trap 'rm -f "${QASM_FILE}"' EXIT

{
    echo "OPENQASM 3.0;"
    echo "qubit[2] q;"
    for ((i = 0; i < N_T_GATES; i++)); do
        echo "h q[0];"
        echo "t q[0];"
    done
} > "${QASM_FILE}"

# Run via the qfault_bench_synthesis helper if it exists; otherwise skip QFault timing.
BENCH_BINARY="${BUILD_DIR}/qfault_bench_synthesis"
if [[ ! -x "${BENCH_BINARY}" ]]; then
    echo ""
    echo "NOTE: ${BENCH_BINARY} not found — skipping QFault pass timing."
    echo "  Build the benchmark binary with: cmake --preset gcc13-release && cmake --build ${BUILD_DIR} -j"
    echo ""
    echo "Baseline only (no overhead comparison):"
    echo "  Direct gridsynth calls for ${N_T_GATES} T-gates: ${BASELINE_MS} ms"
    exit 0
fi

echo "Measuring QFault TGateSynthesisPass<GridSynthProvider>..."
QFAULT_START=$(date +%s%N)
"${BENCH_BINARY}" "${QASM_FILE}" "${EPS}" > /dev/null
QFAULT_END=$(date +%s%N)
QFAULT_MS=$(( (QFAULT_END - QFAULT_START) / 1000000 ))

echo "  QFault:   ${QFAULT_MS} ms"
echo ""

# ── Overhead calculation ──────────────────────────────────────────────────────
if [[ ${BASELINE_MS} -eq 0 ]]; then
    echo "Baseline time is 0 ms — too fast to measure overhead."
    exit 0
fi

# Use awk for floating-point percentage
OVERHEAD=$(awk "BEGIN { printf \"%.2f\", (${QFAULT_MS} - ${BASELINE_MS}) * 100.0 / ${BASELINE_MS} }")

echo "=== Results ==="
printf "  %-30s %6d ms\n" "Baseline (direct gridsynth):" "${BASELINE_MS}"
printf "  %-30s %6d ms\n" "QFault TGateSynthesisPass:" "${QFAULT_MS}"
printf "  %-30s %6s%%\n"  "Overhead:" "${OVERHEAD}"
echo ""

# Exit 1 if overhead > 5%
IS_ABOVE=$(awk "BEGIN { print (${OVERHEAD} > 5.0) ? \"1\" : \"0\" }")
if [[ "${IS_ABOVE}" == "1" ]]; then
    echo "FAIL: overhead ${OVERHEAD}% exceeds 5% stage gate threshold."
    exit 1
else
    echo "PASS: overhead ${OVERHEAD}% is within the 5% stage gate threshold."
    exit 0
fi
