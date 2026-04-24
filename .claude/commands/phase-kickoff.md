# /phase-kickoff — Begin a new stage

Run this at the start of a new stage (after the previous stage gate has passed).

1. Read `docs/phases/stage-N/spec.md` fully (where N is the new stage number)
2. Read `docs/phases/stage-N/todo.md`
3. Read all ADRs flagged as relevant in the spec
4. Run `/pm:prd-new` to create a ccpm PRD for this stage if not done
5. Run `/pm:prd-parse` and `/pm:epic-decompose` to create GitHub Issues
6. Update `memory-bank/activeContext.md`:
   - Set "Current Phase" to the new stage
   - Set "Active Story" to the first story from the stage
   - Set "Next Action" to the first concrete step
7. Update `memory-bank/progress.md`:
   - Set the stage "Started" date
8. Confirm CI is green before writing any stage code
