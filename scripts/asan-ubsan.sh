#!/usr/bin/env bash
set -euo pipefail

ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
BUILD_DIR="$ROOT/build/asan"
CMAKE="${HOME}/.local/bin/cmake"
[[ -x "$CMAKE" ]] || CMAKE="$(command -v cmake)"

echo "=== QFault ASAN + UBSAN ==="
START=$SECONDS

"$CMAKE" --preset asan -S "$ROOT"
"$CMAKE" --build "$BUILD_DIR" -j "$(nproc)"

ASAN_OPTIONS=detect_leaks=1 \
UBSAN_OPTIONS=print_stacktrace=1:halt_on_error=1 \
ctest --test-dir "$BUILD_DIR" --output-on-failure -j "$(nproc)"

echo ""
echo "=== ASAN + UBSAN clean in $((SECONDS - START))s ==="
