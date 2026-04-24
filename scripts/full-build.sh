#!/usr/bin/env bash
set -euo pipefail

ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
CMAKE="${HOME}/.local/bin/cmake"
[[ -x "$CMAKE" ]] || CMAKE="$(command -v cmake)"

echo "=== QFault Full Build (debug + release) ==="
START=$SECONDS

for preset in debug release; do
    echo ""
    echo ">> Preset: $preset"
    "$CMAKE" --preset "$preset" -S "$ROOT"
    "$CMAKE" --build "$ROOT/build/$preset" -j "$(nproc)"
    ctest --test-dir "$ROOT/build/$preset" --output-on-failure -j "$(nproc)"
    echo ">> $preset: OK"
done

echo ""
echo "=== Full build done in $((SECONDS - START))s ==="
