OPENQASM 3.0;
// Adder-4 non-equivalence check: X on first two qubits (encodes input |11..>).
// This is deliberately NOT equivalent to the compiled form to exercise
// QCEC's not-equivalent detection path.
qubit[4] q;
x q[0];
x q[1];
