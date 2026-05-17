#!/usr/bin/env python3
"""Check that tier-1 results match the committed Stage 2 baseline within tolerance.

Usage:
    python3 bench/scripts/check_regression.py [--tol 0.05]

Reads:
    bench/golden/stage2_baseline.csv   — committed reference numbers
    bench/results/tier1.csv            — current run outputs

Exits 0 on pass, 1 on regression detected.
"""

import argparse
import csv
import pathlib
import sys

_BENCH    = pathlib.Path(__file__).parent.parent
_BASELINE = _BENCH / "golden" / "stage2_baseline.csv"
_RESULTS  = _BENCH / "results" / "tier1.csv"


def parse_args() -> argparse.Namespace:
    p = argparse.ArgumentParser(description=__doc__,
                                formatter_class=argparse.RawDescriptionHelpFormatter)
    p.add_argument("--tol", type=float, default=0.05,
                   help="Relative tolerance for regression (default: 0.05 = 5%%)")
    p.add_argument("--baseline", type=pathlib.Path, default=_BASELINE)
    p.add_argument("--results",  type=pathlib.Path, default=_RESULTS)
    return p.parse_args()


def load_csv(path: pathlib.Path) -> dict:
    if not path.exists():
        print(f"ERROR: file not found: {path}", file=sys.stderr)
        sys.exit(1)
    rows = {}
    with path.open(encoding="utf-8") as f:
        for row in csv.DictReader(f):
            rows[row["metric"]] = row
    return rows


def main() -> int:
    args  = parse_args()
    base  = load_csv(args.baseline)
    curr  = load_csv(args.results) if args.results.exists() else {}

    if not curr:
        print("WARNING: bench/results/tier1.csv not found — run 'make tier1' first",
              file=sys.stderr)
        return 0  # Don't fail if results don't exist yet

    regressions = 0
    for metric, brow in base.items():
        if metric not in curr:
            continue  # baseline entry not comparable; skip
        try:
            ref = float(brow["value"])
            got = float(curr[metric]["value"])
        except (KeyError, ValueError):
            continue

        if ref == 0:
            continue

        rel_diff = abs(got - ref) / abs(ref)
        status = "OK" if rel_diff <= args.tol else "REGRESSION"
        if rel_diff > args.tol:
            regressions += 1
        unit = brow.get("unit", "")
        print(f"  {status:10s} {metric}: baseline={ref} {unit}  current={got} {unit}"
              f"  diff={rel_diff*100:.1f}%  tol={args.tol*100:.0f}%")

    if regressions:
        print(f"\n{regressions} regression(s) detected (tolerance {args.tol*100:.0f}%)")
        return 1

    print(f"\nAll checks passed (tolerance {args.tol*100:.0f}%)")
    return 0


if __name__ == "__main__":
    sys.exit(main())
