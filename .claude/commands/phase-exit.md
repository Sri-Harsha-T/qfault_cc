# /phase-exit — Complete a stage and pass the gate

Run this when all stories in a stage are done and you want to formally close it.

1. Run the full stage gate checklist from `docs/phases/stage-N/spec.md`
   "Stage Gate" section. Each criterion must be verifiable — link to test output
   or commit hash.

2. Run all quality gates:
   ```bash
   ./scripts/full-build.sh
   ./scripts/asan-ubsan.sh
   clang-tidy --config-file=.clang-tidy src/
   ctest -j --output-on-failure
   ```

3. Write a `docs/phases/stage-N/exit-report.md`:
   ```markdown
   # Stage N Exit Report
   **Date:** YYYY-MM-DD
   **Gate result:** PASSED / FAILED (explain if failed)

   ## Deliverables
   | Deliverable | Status | Evidence |
   |-------------|--------|---------|
   | ... | ✅ Done | commit abc1234 |

   ## Gate Criteria
   | Criterion | Result | Evidence |
   |-----------|--------|---------|
   | ... | ✅ | test output / benchmark numbers |

   ## New ADRs Created This Stage
   | ADR | Title |
   |-----|-------|

   ## Known Limitations Carried Forward
   - ...

   ## Failed Approaches Discovered
   - (Also logged in CHANGELOG.md)
   ```

4. Tag a release: `git tag v0.N.0-stageN && git push --tags`

5. Update `memory-bank/progress.md` — mark stage as complete with date.

6. Close the GitHub Milestone for this stage.

7. Run `/phase-kickoff` for the next stage.
