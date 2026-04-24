# /log — End-of-session state capture

Before ending this session (before /clear or /compact), do the following:

1. Update `memory-bank/activeContext.md`:
   - Set "_Last updated_" to today's date
   - Update "State of Work" checkboxes based on what was done
   - Update "Recent Decisions" if any decisions were made
   - Update "Open Blockers" if any new blockers were found
   - **Update "Next Action"** — this is the most important field. Be specific:
     include the file, function, or line number where work should resume.
   - Update "Files Touched This Session"

2. Update `CHANGELOG.md`:
   - If any approach was tried and found not to work, add an entry to
     "Failed Approaches" with: date, what was tried, why it failed, what replaced it
   - Add a brief entry to the Stage Progress Log if a story was completed

3. Update `docs/phases/stage-N/todo.md`:
   - Mark completed items with [x]

4. If any new ADRs were created or updated, confirm `docs/adr/README.md` index
   is current.

5. Confirm: `./scripts/quick-test.sh` is green before ending the session.

After completing all of the above, summarise:
- What was accomplished
- What is next (verbatim from "Next Action" in activeContext.md)
- Any blockers to be aware of
