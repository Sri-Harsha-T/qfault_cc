#!/usr/bin/env bash
set -euo pipefail

ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"

# Use gcc13-debug by default; override with QFAULT_PRESET env var.
# The generic 'debug' preset uses system gcc 9.4 which cannot compile C++20.
PRESET="${QFAULT_PRESET:-gcc13-debug}"
BUILD_DIR="$ROOT/build/${PRESET}"

# Prefer pip-installed cmake (3.21+ required for preset support)
CMAKE="${HOME}/.local/bin/cmake"
[[ -x "$CMAKE" ]] || CMAKE="$(command -v cmake)"

echo "=== QFault Quick Test (preset: ${PRESET}, cmake: $($CMAKE --version | head -1)) ==="
START=$SECONDS

# Configure only if build directory doesn't exist
if [[ ! -f "$BUILD_DIR/CMakeCache.txt" ]]; then
    echo ">> Configuring ${PRESET} build..."
    "$CMAKE" --preset "${PRESET}" -S "$ROOT"
fi

echo ">> Building..."
"$CMAKE" --build "$BUILD_DIR" -j "$(nproc)"

echo ">> Running tests..."
"$CMAKE" --build "$BUILD_DIR" --target test -- CTEST_OUTPUT_ON_FAILURE=1

echo ""
echo "=== Done in $((SECONDS - START))s ==="
