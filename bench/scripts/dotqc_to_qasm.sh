#!/usr/bin/env bash
# dotqc_to_qasm.sh — Convert Feynman/Ross-Selinger .qc format to QASM 3.0.
#
# Usage:
#   ./bench/scripts/dotqc_to_qasm.sh INPUT.qc [OUTPUT.qasm]
#
# If OUTPUT.qasm is omitted, the result is written to stdout.
#
# .qc format (subset handled here):
#   .v q0 q1 ... qn           — qubit variable declarations
#   .i q0 q1 ...              — input qubits (subset of .v)
#   .o q0 q1 ...              — output qubits (ignored for gate-level emit)
#   BEGIN                     — start of gate list
#   <gate> <qubits>           — gates (H, cnot, T, T*, S, S*, X, Z, tof)
#   END                       — end of gate list
#
# Gate mapping (Feynman → QASM 3.0):
#   H   → h
#   X   → x
#   Z   → z
#   S   → s
#   S*  → sdg
#   T   → t
#   T*  → tdg
#   cnot ctrl tgt → cx ctrl, tgt
#   tof c1 c2 tgt → ccx c1, c2, tgt  (Toffoli)
#
# Limitations:
#   - No ancilla qubit handling
#   - Only single-wire and two/three-wire gates above
#   - Comments (#) stripped; blank lines ignored

set -euo pipefail

INPUT="${1:-}"
OUTPUT="${2:-/dev/stdout}"

if [[ -z "$INPUT" ]]; then
    echo "Usage: $0 INPUT.qc [OUTPUT.qasm]" >&2
    exit 1
fi

if [[ ! -f "$INPUT" ]]; then
    echo "ERROR: file not found: $INPUT" >&2
    exit 1
fi

awk '
BEGIN {
    in_gates = 0
    n_qubits = 0
    header_done = 0
}

# Strip comments and trim
{
    sub(/#.*/, "")
    gsub(/^[ \t]+|[ \t]+$/, "")
    if ($0 == "") next
}

/^\.v / {
    split($0, parts, /[ \t]+/)
    n_qubits = length(parts) - 1
    if (!header_done) {
        print "OPENQASM 3.0;"
        printf "qubit[%d] q;\n", n_qubits
        # Build qubit name → index map
        for (i = 2; i <= length(parts); i++) {
            qmap[parts[i]] = i - 2
        }
        header_done = 1
    }
    next
}

/^\.i / || /^\.o / || /^\.c / { next }

/^BEGIN$/ { in_gates = 1; next }
/^END$/ { in_gates = 0; next }

in_gates {
    gate = $1
    if (gate == "H" || gate == "h") {
        printf "h q[%d];\n", qmap[$2]
    } else if (gate == "X" || gate == "x") {
        printf "x q[%d];\n", qmap[$2]
    } else if (gate == "Z" || gate == "z") {
        printf "z q[%d];\n", qmap[$2]
    } else if (gate == "S") {
        printf "s q[%d];\n", qmap[$2]
    } else if (gate == "S*" || gate == "Sd" || gate == "sdg") {
        printf "sdg q[%d];\n", qmap[$2]
    } else if (gate == "T") {
        printf "t q[%d];\n", qmap[$2]
    } else if (gate == "T*" || gate == "Td" || gate == "tdg") {
        printf "tdg q[%d];\n", qmap[$2]
    } else if (gate == "cnot" || gate == "CNOT" || gate == "cx") {
        printf "cx q[%d], q[%d];\n", qmap[$2], qmap[$3]
    } else if (gate == "tof" || gate == "ccx" || gate == "CCX") {
        printf "ccx q[%d], q[%d], q[%d];\n", qmap[$2], qmap[$3], qmap[$4]
    } else {
        print "// UNSUPPORTED: " $0 > "/dev/stderr"
    }
}
' "$INPUT" > "$OUTPUT"
