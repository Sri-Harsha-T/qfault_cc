#!/usr/bin/env python3
"""Generate MQT Bench circuits and write them to bench/_generated/.

Usage:
    python3 bench/scripts/gen_mqtbench.py [--level {alg,nativegates,mapped}]
                                          [--min-qubits N] [--max-qubits N]
                                          [--benchmarks NAME [NAME ...]]

Requires: mqt-bench (pip install mqt-bench)
Output:   bench/_generated/mqtbench/<benchmark>_n<qubits>.qasm

This script is the single entry point for regenerating MQT Bench circuits.
The generated files are gitignored — regenerate on any system with:
    pip install mqt-bench
    python3 bench/scripts/gen_mqtbench.py
"""

import argparse
import pathlib
import sys

_GENERATED = pathlib.Path(__file__).parent.parent / "_generated" / "mqtbench"

_DEFAULT_BENCHMARKS = [
    "ae",          # Amplitude Estimation
    "dj",          # Deutsch-Jozsa
    "ghz",         # GHZ state preparation
    "graphstate",  # Graph state
    "grover",      # Grover's algorithm
    "qaoa",        # Quantum Approximate Optimisation
    "qft",         # Quantum Fourier Transform
    "qnn",         # Quantum Neural Network
    "qpe",         # Quantum Phase Estimation
    "qwalk",       # Quantum Walk
    "su2random",   # Random SU(2) circuit
    "twolocalrandom",  # Two-local random ansatz
    "vqe",         # Variational Quantum Eigensolver
    "wstate",      # W state preparation
]


def parse_args() -> argparse.Namespace:
    p = argparse.ArgumentParser(description=__doc__,
                                formatter_class=argparse.RawDescriptionHelpFormatter)
    p.add_argument("--level", choices=["alg", "nativegates", "mapped"],
                   default="alg",
                   help="Abstraction level to generate (default: alg)")
    p.add_argument("--min-qubits", type=int, default=2,
                   help="Smallest qubit count to generate (default: 2)")
    p.add_argument("--max-qubits", type=int, default=10,
                   help="Largest qubit count to generate (default: 10)")
    p.add_argument("--benchmarks", nargs="+", default=_DEFAULT_BENCHMARKS,
                   metavar="NAME",
                   help="Subset of MQT Bench benchmark names (default: all)")
    p.add_argument("--dry-run", action="store_true",
                   help="Print what would be generated without writing files")
    return p.parse_args()


def main() -> int:
    args = parse_args()

    try:
        from mqt.bench import get_benchmark  # type: ignore[import]
    except ImportError:
        print("ERROR: mqt-bench not installed. Run: pip install mqt-bench",
              file=sys.stderr)
        return 1

    _GENERATED.mkdir(parents=True, exist_ok=True)

    generated = 0
    failed = 0

    for bench in args.benchmarks:
        for n in range(args.min_qubits, args.max_qubits + 1):
            outpath = _GENERATED / f"{bench}_n{n:02d}.qasm"
            if args.dry_run:
                print(f"[dry-run] would write {outpath}")
                continue
            try:
                qc = get_benchmark(bench, level=args.level, circuit_size=n)
                qasm_str = qc.qasm()
                outpath.write_text(qasm_str, encoding="utf-8")
                print(f"  wrote {outpath.name}  ({len(qasm_str)} bytes)")
                generated += 1
            except Exception as exc:  # noqa: BLE001
                print(f"  SKIP {bench} n={n}: {exc}", file=sys.stderr)
                failed += 1

    if not args.dry_run:
        print(f"\nDone: {generated} written, {failed} skipped.")
        print(f"Output directory: {_GENERATED}")

    return 0 if failed == 0 else 2


if __name__ == "__main__":
    sys.exit(main())
