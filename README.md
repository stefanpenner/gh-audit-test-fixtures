# gh-audit-test-fixtures

Test fixture repo for [gh-audit](https://github.com/stefanpenner/gh-audit). `scenarios.json` is the machine-readable index of scenario IDs → branch names, commit SHAs, and expected audit outcomes. It's consumed by `scripts/test-fixtures.sh` in the gh-audit repo to validate each release.

## Series

- **1.x** — merge strategies and multi-reviewer scenarios
- **2.x** — missing/weak approval paths
- **3.x** — dismissed / comment-only reviews
- **4.x** — revert scenarios (see below)

## Revert scenarios (4.x)

Map 1:1 to the revert-classification table in gh-audit's `Architecture.md` rule 8.

| # | Title | Compliant? | Why |
|---|---|---|---|
| 4.1 | AutoRevert (bot-style message) | yes | R1 (trusted by construction) |
| 4.2 | Clean `git revert` | yes | R1 (diff-verified via `This reverts commit` trailer) |
| 4.3 | `git revert` with conflict resolution | no | `diff-mismatch` |
| 4.4 | Manual revert with stripped trailer | no | `message-only` — no reverted SHA recoverable |
| 4.5 | GH "Revert" button, squash-merged | yes | R1 — trailer missing, SHA recovered via `revert-<N>-…` head-branch fallback |
| 4.6 | GH "Revert" button with conflicts | — | not reproducible; GH refuses the button when the target can't be auto-reverted |
| 4.7 | GH "Revert" button, merge-commit strategy | yes | R1 on the underlying revert commit in the second-parent chain |
| 4.8 | Revert-of-revert via GH UI ⚠️ | no | `RevertOfRevert` is never classified clean |
| 4.9 | Revert-of-revert via local `git revert` (legacy message) | no | same — `RevertOfRevert` never clean |
| 4.10 | Direct-push clean revert (no PR) | **no** | rule 3 (no associated PR) wins over rule 8; bypassing PR review is a violation even with a clean diff |
| 4.11 | Octopus merge | no | not revert-related; `OctopusMerge` kind |
| 4.12 | Empty revert of empty base | yes | empty-commit fallback fires first (R1 would also cover it) |

⚠️ = requires manual completion via the GH UI. The only remaining UI-only scenario is 4.8 — 4.5 and 4.7 are already produced and indexed.

## Completing scenario 4.8

1. Scenario 4.5 must already be complete (its revert [PR #33](https://github.com/stefanpenner/gh-audit-test-fixtures/pull/33) exists and is merged).
2. Open [PR #33](https://github.com/stefanpenner/gh-audit-test-fixtures/pull/33) on github.com.
3. Click the **Revert** button on that revert PR. GitHub will open a new "Revert Revert …" PR.
4. Merge with **Squash and merge**.
5. The resulting commit's message should start with `Revert "Revert \"Scenario 4.5: …\""`. gh-audit classifies it as `RevertOfRevert` and falls through to rule 7.
6. Add an entry to `scenarios.json` under 4.8's `assertions` with the new commit SHA, `is_compliant: false`, `is_clean_revert: false`, `revert_verification: "none"`.

## Scenarios intentionally not reproducible

- **4.6** — GitHub's "Revert" button refuses when the target PR can no longer be auto-reverted (error: *"Sorry, this pull request couldn't be reverted automatically…"*). A developer hitting that falls back to local `git revert` with manual conflict resolution, which scenario 4.3 already covers.
