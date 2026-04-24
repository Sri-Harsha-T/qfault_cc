# /adr — Consult and Create Architecture Decision Records

Before planning or implementing anything that involves:
- Adding a new dependency
- Changing the IR schema or data structures
- Switching or adding a synthesis algorithm
- Changing a numerical convention (code distance, gate angles, Pauli conventions)
- Changing the output format spec

Do the following:

1. Read `docs/adr/README.md` (the index table)
2. Read any ADRs listed as relevant to the current task
3. If the current plan CONTRADICTS an accepted ADR, stop and flag the conflict
   before proceeding. Ask for a decision before writing code.
4. If the current plan requires a NEW architectural decision, propose a new ADR
   using `docs/adr/template.md`. Assign the next sequential number.
5. If a plan would make an existing ADR obsolete, propose an update to its status
   (Deprecated or Superseded) and the new ADR simultaneously.

After reading ADRs, proceed with planning. Include in your plan summary:
- Which ADRs are relevant
- Any conflicts detected
- Any new ADRs proposed

Do not start implementation until ADR conflicts are resolved.
