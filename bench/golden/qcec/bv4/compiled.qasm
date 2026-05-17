OPENQASM 3.0;
// BV-4: equivalent to original — CX oracle gates in descending qubit order.
// CX_{i,ancilla} gates commute (same target), so order is irrelevant.
qubit[5] q;
h q[0];
h q[1];
h q[2];
h q[3];
x q[4];
h q[4];
cx q[3], q[4];
cx q[2], q[4];
cx q[1], q[4];
cx q[0], q[4];
h q[0];
h q[1];
h q[2];
h q[3];
