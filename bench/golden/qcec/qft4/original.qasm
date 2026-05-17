OPENQASM 3.0;
// QFT-4 simplified reference: H-and-CNOT network on 4 qubits.
// This models the entanglement structure of QFT without phase rotations.
// Original: CNOT cascade forward then H on each qubit.
qubit[4] q;
h q[0];
h q[1];
h q[2];
h q[3];
cx q[0], q[1];
cx q[1], q[2];
cx q[2], q[3];
cx q[2], q[3];
cx q[1], q[2];
cx q[0], q[1];
h q[0];
h q[1];
h q[2];
h q[3];
