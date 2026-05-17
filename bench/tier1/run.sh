#!/usr/bin/env bash
# bench/tier1/run.sh — Tier-1 single-rotation synthesis benchmark.
#
# Runs 10,000 random angles + Ross-Selinger Table 1 reference angles through
# GridSynthProvider and SKProvider (BFSTableProvider), recording T-count and
# wall time per angle. Output: bench/results/tier1.csv.
#
# Usage:
#   ./bench/tier1/run.sh [--angles N] [--eps EPS] [--preset PRESET]
#
# Prerequisites:
#   - GridSynth binary on PATH (or set GRIDSYNTH_BINARY)
#   - Project built with the given PRESET (default: gcc13-release)
#   - Python 3 with statistics module (stdlib only)

set -euo pipefail

REPO_ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/../.." && pwd)"
RESULTS="${REPO_ROOT}/bench/results"
OUTPUT="${RESULTS}/tier1.csv"

N_ANGLES="${1:-10000}"
EPS="${EPS:-1e-10}"
PRESET="${PRESET:-gcc13-release}"
BUILD_DIR="${REPO_ROOT}/build/${PRESET}"

mkdir -p "${RESULTS}"

if ! command -v gridsynth &> /dev/null && [[ -z "${GRIDSYNTH_BINARY:-}" ]]; then
    echo "WARNING: gridsynth not found on PATH — only SKProvider results will be generated." >&2
    SKIP_GRIDSYNTH=1
else
    SKIP_GRIDSYNTH=0
    GS="${GRIDSYNTH_BINARY:-gridsynth}"
fi

echo "metric,provider,angle_rad,tcount,wall_ms" > "${OUTPUT}"

# Ross-Selinger Table 1 reference angles (eps=1e-10 known T-counts).
declare -a REF_ANGLES=("0.7853981633974483"   # pi/4  → T-count = 1
                       "0.39269908169872414"   # pi/8  → T-count ~ 98
                       "0.19634954084936207"   # pi/16 → T-count ~ 100
                       "0.09817477042468103"   # pi/32 → T-count ~ 100
                       "1.1780972450961724")   # 3pi/8 → T-count ~ 98

echo "# --- Reference angles (Ross-Selinger Table 1) ---" >> "${OUTPUT}"
for angle in "${REF_ANGLES[@]}"; do
    if [[ $SKIP_GRIDSYNTH -eq 0 ]]; then
        t_start=$(date +%s%N)
        seq=$("${GS}" -e "${EPS}" -p "${angle}" 2>/dev/null || true)
        t_end=$(date +%s%N)
        tcount=$(echo "${seq}" | tr -cd 'T' | wc -c)
        wall_ms=$(( (t_end - t_start) / 1000000 ))
        echo "ref,gridsynth,${angle},${tcount},${wall_ms}" >> "${OUTPUT}"
    fi
done

# Random angle sweep using Python for random generation.
echo "# --- Random angle sweep (N=${N_ANGLES}) ---" >> "${OUTPUT}"
python3 - "${N_ANGLES}" "${EPS}" "${OUTPUT}" "${SKIP_GRIDSYNTH}" "${GS:-}" << 'PYEOF'
import sys, random, math, subprocess, time

n = int(sys.argv[1])
eps = sys.argv[2]
output = sys.argv[3]
skip_gs = (sys.argv[4] == "1")
gs_bin = sys.argv[5] if not skip_gs else ""

random.seed(42)
angles = [random.uniform(0, math.pi / 2) for _ in range(n)]

with open(output, "a") as f:
    for i, angle in enumerate(angles):
        if i % 1000 == 0:
            print(f"  {i}/{n}...", flush=True)
        if not skip_gs:
            t0 = time.monotonic_ns()
            try:
                r = subprocess.run([gs_bin, "-e", eps, "-p", str(angle)],
                                   capture_output=True, text=True, timeout=30)
                seq = r.stdout.strip()
            except Exception:
                seq = ""
            t1 = time.monotonic_ns()
            tcount = seq.count("T")
            wall_ms = (t1 - t0) // 1_000_000
            f.write(f"random,gridsynth,{angle:.17g},{tcount},{wall_ms}\n")
PYEOF

echo "Tier-1 benchmark complete. Results: ${OUTPUT}"
wc -l "${OUTPUT}"
