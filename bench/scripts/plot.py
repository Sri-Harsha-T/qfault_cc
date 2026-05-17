#!/usr/bin/env python3
"""Plot QFault Stage 2.5 benchmark results.

Reads bench/results/tier1.csv and bench/results/tier2.csv,
produces PDFs under bench/plots/.

Usage:
    python3 bench/scripts/plot.py [--results-dir DIR] [--plots-dir DIR]

Requires: matplotlib, pandas
"""

import argparse
import pathlib
import sys

_BENCH = pathlib.Path(__file__).parent.parent
_RESULTS = _BENCH / "results"
_PLOTS   = _BENCH / "plots"


def parse_args() -> argparse.Namespace:
    p = argparse.ArgumentParser(description=__doc__,
                                formatter_class=argparse.RawDescriptionHelpFormatter)
    p.add_argument("--results-dir", type=pathlib.Path, default=_RESULTS)
    p.add_argument("--plots-dir",   type=pathlib.Path, default=_PLOTS)
    p.add_argument("--show", action="store_true", help="Show interactive window")
    return p.parse_args()


def _require_imports():
    try:
        import matplotlib.pyplot as plt  # noqa: F401
        import pandas as pd              # noqa: F401
    except ImportError as e:
        print(f"ERROR: missing dependency — {e}\n"
              "Install with: pip install matplotlib pandas", file=sys.stderr)
        sys.exit(1)


def plot_tier1(results_dir: pathlib.Path, plots_dir: pathlib.Path, show: bool) -> None:
    import matplotlib.pyplot as plt
    import pandas as pd

    csv = results_dir / "tier1.csv"
    if not csv.exists():
        print(f"tier1.csv not found at {csv} — skipping tier-1 plots", file=sys.stderr)
        return

    df = pd.read_csv(csv)
    required = {"provider", "angle_rad", "tcount", "wall_ms"}
    if not required.issubset(df.columns):
        print(f"WARNING: tier1.csv missing columns {required - set(df.columns)}", file=sys.stderr)
        return

    fig, axes = plt.subplots(1, 2, figsize=(12, 5))

    # T-count distribution per provider
    df.boxplot(column="tcount", by="provider", ax=axes[0])
    axes[0].set_title("T-count distribution by provider")
    axes[0].set_xlabel("Provider")
    axes[0].set_ylabel("T-count")

    # Wall time distribution per provider
    df.boxplot(column="wall_ms", by="provider", ax=axes[1])
    axes[1].set_title("Wall time distribution by provider (ms)")
    axes[1].set_xlabel("Provider")
    axes[1].set_ylabel("Wall time (ms)")

    plt.suptitle("QFault Tier-1 Benchmark (Stage 2.5)")
    plt.tight_layout()

    out = plots_dir / "tier1_summary.pdf"
    plots_dir.mkdir(parents=True, exist_ok=True)
    fig.savefig(out)
    print(f"Wrote {out}")

    if show:
        plt.show()
    plt.close(fig)


def plot_tier2(results_dir: pathlib.Path, plots_dir: pathlib.Path, show: bool) -> None:
    import matplotlib.pyplot as plt
    import pandas as pd

    csv = results_dir / "tier2.csv"
    if not csv.exists():
        print(f"tier2.csv not found at {csv} — skipping tier-2 plots", file=sys.stderr)
        return

    df = pd.read_csv(csv)
    required = {"benchmark", "tcount", "gate_count", "depth_cycles", "compile_ms"}
    if not required.issubset(df.columns):
        print(f"WARNING: tier2.csv missing columns {required - set(df.columns)}", file=sys.stderr)
        return

    fig, ax = plt.subplots(figsize=(10, 6))
    df_sorted = df.sort_values("tcount", ascending=True)
    ax.barh(df_sorted["benchmark"], df_sorted["tcount"])
    ax.set_xlabel("T-count after QFault synthesis pass")
    ax.set_title("QFault Tier-2 Benchmark: T-count by circuit (Stage 2.5)")
    ax.invert_yaxis()

    plt.tight_layout()
    out = plots_dir / "tier2_tcount.pdf"
    plots_dir.mkdir(parents=True, exist_ok=True)
    fig.savefig(out)
    print(f"Wrote {out}")

    if show:
        plt.show()
    plt.close(fig)


def main() -> int:
    args = parse_args()
    _require_imports()
    plot_tier1(args.results_dir, args.plots_dir, args.show)
    plot_tier2(args.results_dir, args.plots_dir, args.show)
    return 0


if __name__ == "__main__":
    sys.exit(main())
