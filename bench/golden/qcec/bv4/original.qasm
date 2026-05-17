OPENQASM 3.0;
// BV-4: Bernstein-Vazirani with secret s=1111, ancilla = q[4].
// CX oracle gates in ascending qubit order.
qubit[5] q;
h q[0];
h q[1];
h q[2];
h q[3];
x q[4];
h q[4];
cx q[0], q[4];
cx q[1], q[4];
cx q[2], q[4];
cx q[3], q[4];
h q[0];
h q[1];
h q[2];
h q[3];
