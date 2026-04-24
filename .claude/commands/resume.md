# /resume — Start a new Claude Code session for QFault

Read the following files in order to restore context for this session:
1. `CLAUDE.md` — project conventions and rules
2. `memory-bank/activeContext.md` — current stage, active story, next action
3. `CHANGELOG.md` — specifically the "Failed Approaches" section
4. `memory-bank/progress.md` — current stage status

After reading, confirm:
- Which stage you are in (1–5)
- The active story (GitHub Issue number if known)
- The exact "Next Action" from activeContext.md
- Any open blockers

Then run `./scripts/quick-test.sh` to confirm the test baseline is green before
making any changes.

Do NOT start coding before confirming the baseline is green and the next action
is clear.
