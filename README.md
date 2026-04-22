# gh-audit-test-fixtures

Test fixture repo for [gh-audit](https://github.com/stefanpenner/gh-audit). Each scenario is tagged for automated validation. `scenarios.json` is the index of scenario IDs → branch names, SHAs, and expected verdicts.

## Series

- **1.x** — merge strategies and multi-reviewer scenarios
- **2.x** — missing/weak approval paths
- **3.x** — dismissed / comment-only reviews
- **4.x** — revert scenarios (see below)

## Revert scenarios (4.x)

Map 1:1 to the revert-classification table in gh-audit's `Architecture.md`.

| # | Title | Waived? | Rule |
|---|---|---|---|
| 4.1 | AutoRevert (bot-style message) | yes | R1 (trusted by construction) |
| 4.2 | Clean `git revert` | yes | R1 (diff-verified) |
| 4.3 | `git revert` with conflict resolution | no | falls through |
| 4.4 | Manual revert with stripped trailer | no | falls through |
| 4.5 | GH "Revert" button, clean, squash/rebase-merged ⚠️ | yes | R1 (diff-verified) |
| 4.6 | GH "Revert" button with conflict resolution ⚠️ | no | falls through |
| 4.7 | GH "Revert" button, merge-commit strategy ⚠️ | no | merge commit isn't a revert |
| 4.8 | Revert-of-revert via GH UI ⚠️ | no | RevertOfRevert never clean |
| 4.9 | Revert-of-revert via local `git revert` (legacy message) | no | RevertOfRevert never clean |
| 4.10 | Direct-push revert (no PR) | yes | R1 applies without a PR |
| 4.11 | Octopus merge | no | not revert-related; OctopusMerge kind |
| 4.12 | Empty revert of empty base | yes | R1 (trivially) + empty-commit fallback |

⚠️ = requires manual completion via the GH UI; scripting the "Revert" button isn't possible.

## Completing the UI-only scenarios

These four scenarios can only be produced by clicking buttons on github.com — the committer has to be `web-flow` and the signature has to be GitHub-generated, which no local flow can reproduce. Follow them in order; each step assumes the previous one is complete.

### 4.5 — Clean revert via GH "Revert" button (squash-merged)

1. Create a base PR:
   - Branch: `scenario/4.5-gh-revert-clean-base` off `main`
   - Add file `scenario-4.5.txt` with contents `4.5 base`
   - Open PR "Scenario 4.5: base (for GH revert button)" and merge it (any strategy).
2. Browse to the merged PR on github.com.
3. Click the **Revert** button at the bottom. GitHub creates `revert-<N>-scenario/4.5-gh-revert-clean-base` and opens PR "Revert Scenario 4.5: base (for GH revert button)".
4. Merge that new PR with **Squash and merge**.
5. On the squashed revert commit, verify on github.com that:
   - Committer is `web-flow`
   - Signature shows "Verified"
   - Message starts with `Revert "Scenario 4.5: …"` and contains the `This reverts commit …` trailer.
6. Update `scenarios.json` with the resulting branch name and revert-commit SHA.

### 4.6 — GH "Revert" button with conflict resolution

1. Create and merge a base PR `scenario/4.6-gh-revert-conflict-base` that adds `scenario-4.6.txt` with three lines `aaa\nbbb\nccc\n`.
2. Create and merge a second PR `scenario/4.6-gh-revert-conflict-modify` that changes line 2 from `bbb` to `BBB`. This is what will cause the revert to conflict.
3. Browse to the base PR on github.com and click **Revert**.
4. GitHub will offer to open a PR; the revert branch will have conflicts. Resolve them in the GH web editor (e.g., keep `BBB`, delete `aaa` and `ccc`).
5. Merge the conflict-resolved revert PR (any strategy).
6. Update `scenarios.json`.

### 4.7 — GH "Revert" button, merge-commit strategy

1. Create and merge a base PR `scenario/4.7-gh-revert-merge-base` that adds `scenario-4.7.txt`.
2. Click the **Revert** button on the merged PR.
3. When merging the resulting revert PR, pick **Create a merge commit** from the split-button merge menu.
4. On main you should now see a 2-parent `Merge pull request #…` commit whose second-parent chain contains the revert. gh-audit should classify the merge commit itself as `CleanMerge` (not a revert).
5. Update `scenarios.json`.

### 4.8 — Revert-of-revert via the GH UI

1. Complete scenario 4.5 first (or reuse its branches).
2. Browse to the **revert** PR created by 4.5 on github.com.
3. Click the **Revert** button on *that* PR.
4. Merge the resulting re-apply PR with Squash and merge.
5. The squashed commit should have a message like `Revert "Revert \"Scenario 4.5: …\""`, committer=`web-flow`, verified. gh-audit classifies it as `RevertOfRevert` — never clean, falls through.
6. Update `scenarios.json`.
