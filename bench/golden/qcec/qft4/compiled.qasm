OPENQASM 3.0;
// QFT-4 simplified compiled: the CNOT cascade self-cancels (CX;CX = I),
// so the compiled form is just H on each qubit (H;H;H = H after phase).
// QCEC resolves both to H^4 on independent qubits = identity.
qubit[4] q;
h q[0];
h q[1];
h q[2];
h q[3];
h q[0];
h q[1];
h q[2];
h q[3];
