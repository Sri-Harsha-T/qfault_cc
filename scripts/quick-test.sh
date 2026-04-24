#!/usr/bin/env bash
set -euo pipefail

ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
BUILD_DIR="$ROOT/build/debug"

# Prefer pip-installed cmake (3.21+ required for preset support)
CMAKE="${HOME}/.local/bin/cmake"
[[ -x "$CMAKE" ]] || CMAKE="$(command -v cmake)"

echo "=== QFault Quick Test (cmake: $($CMAKE --version | head -1)) ==="
START=$SECONDS

# Configure only if build directory doesn't exist
if [[ ! -f "$BUILD_DIR/CMakeCache.txt" ]]; then
    echo ">> Configuring debug build..."
    "$CMAKE" --preset debug -S "$ROOT"
fi

echo ">> Building..."
"$CMAKE" --build "$BUILD_DIR" -j "$(nproc)"

echo ">> Running tests..."
"$CMAKE" --build "$BUILD_DIR" --target test -- CTEST_OUTPUT_ON_FAILURE=1

echo ""
echo "=== Done in $((SECONDS - START))s ==="
