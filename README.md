# gh-audit-test-fixtures

Test fixture repo for [gh-audit](https://github.com/stefanpenner/gh-audit). `scenarios.json` is the machine-readable index of scenario IDs → branch names, commit SHAs, and expected audit outcomes. It's consumed by `scripts/test-fixtures.sh` in the gh-audit repo to validate each release.

## Series

- **[1.x](#1x--merge-strategies)** — Merge strategies
- **[2.x](#2x--missingweak-approval)** — Missing/weak approval paths
- **[3.x](#3x--dismissed--comment-only-reviews)** — Dismissed / comment-only reviews
- **[4.x](#4x--revert-scenarios)** — Revert scenarios
- **[5.x](#5x--data-shape-edge-cases)** — Data-shape edge cases

---

## 1.x — Merge strategies

Tests commit→PR linking across GitHub's three merge strategies and verifies merge classification.

| # | Title | PR | Compliant? | Why |
|---|---|---|---|---|
| 1.1 | Squash merge — no review | [#36](https://github.com/stefanpenner/gh-audit-test-fixtures/pull/36) | no | no approval on final commit |
| 1.2 | Merge commit — no review | [#37](https://github.com/stefanpenner/gh-audit-test-fixtures/pull/37) | no | no approval; also tests `is_clean_merge=true` (verified web-flow signature) |
| 1.3 | Rebase merge — no review | [#38](https://github.com/stefanpenner/gh-audit-test-fixtures/pull/38) | no | no approval; tests rebased SHA→PR linking |
| 1.4 | Dirty merge — local `--no-ff` pushed directly | — | no | 2 parents, non-web-flow committer, non-PR message → `DirtyMerge`; never auto-trusted |

## 2.x — Missing/weak approval

Tests rule 4 (approval on final commit) failure modes.

| # | Title | PR | Compliant? | Why |
|---|---|---|---|---|
| 2.1 | No approval — PR merged without reviews | [#39](https://github.com/stefanpenner/gh-audit-test-fixtures/pull/39) | no | zero reviews of any kind |

### Scenarios requiring a second GitHub user

The following scenarios need an independent reviewer (not the PR author) to test approval paths. They cannot be created with a single GitHub account because GitHub blocks self-approval via API.

- **2.2** — Self-approval on final commit → `is_self_approved=true`
- **2.3** — Stale approval (approved on old SHA, new commit pushed) → `has_stale_approval=true`
- **2.4** — Independent approval on final commit → `is_compliant=true` (the happy path)
- **2.5** — Post-merge concern (CHANGES_REQUESTED after merge) → `has_post_merge_concern=true`

## 3.x — Dismissed / comment-only reviews

Tests that non-state-changing reviews don't count as approval.

| # | Title | PR | Compliant? | Why |
|---|---|---|---|---|
| 3.1 | COMMENT-only review | [#40](https://github.com/stefanpenner/gh-audit-test-fixtures/pull/40) | no | COMMENT reviews are informational — they don't constitute approval |

### Scenarios requiring a second GitHub user

- **3.2** — APPROVE then DISMISS by same reviewer → `has_final_approval=false`
- **3.3** — APPROVED then COMMENTED (approval survives) → `is_compliant=true`

## 4.x — Revert scenarios

Map 1:1 to the revert-classification table in gh-audit's `Architecture.md` rule 8.

| # | Title | PR | Compliant? | Why |
|---|---|---|---|---|
| 4.1 | AutoRevert (bot-style message) | — | yes | R1 (trusted by construction) |
| 4.2 | Clean `git revert` | — | yes | R1 (diff-verified via `This reverts commit` trailer) |
| 4.3 | `git revert` with conflict resolution | — | no | `diff-mismatch` |
| 4.4 | Manual revert with stripped trailer | — | no | `message-only` — no reverted SHA recoverable |
| 4.5 | GH "Revert" button, squash-merged | [#33](https://github.com/stefanpenner/gh-audit-test-fixtures/pull/33) | yes | R1 — SHA recovered via `revert-<N>-…` head-branch fallback |
| 4.6 | GH "Revert" button with conflicts | — | — | not reproducible; GH refuses the button |
| 4.7 | GH "Revert" button, merge-commit strategy | [#34](https://github.com/stefanpenner/gh-audit-test-fixtures/pull/34) | yes | R1 on underlying commit in second-parent chain |
| 4.8 | Revert-of-revert via GH UI | [#35](https://github.com/stefanpenner/gh-audit-test-fixtures/pull/35) | no | `RevertOfRevert` never classified clean |
| 4.9 | Revert-of-revert via local `git revert` | — | no | same — `RevertOfRevert` never clean |
| 4.10 | Direct-push clean revert (no PR) | — | yes | rule 8 waives any non-compliant primary verdict, including "no PR" — the diff is a verified pure inverse |
| 4.11 | Octopus merge (3-way merge) | — | no | `OctopusMerge` kind; not auto-classified |
| 4.12 | Empty revert of empty base | — | yes | empty-commit fallback fires first |
| 4.13 | Clean revert of `++`/`--` content lines | — | yes | hunk-aware patch parsing must not drop `+++…`/`---…`-serialized content as headers |
| 4.14 | Large revert (>300 files) | — | yes | `files[]` pagination; truncated comparisons must never produce `diff-verified` |

## 5.x — Data-shape edge cases

Inputs that historically broke ingestion or reporting rather than the rules themselves.

| # | Title | PR | Compliant? | Why |
|---|---|---|---|---|
| 5.1 | Empty commit, direct push | — | yes | rule 2 waives before rule 3; `is_empty_commit=true` |
| 5.2 | Hostile commit message (`=HYPERLINK(…)`, `+cmd`, tabs) | — | no | verdict is ordinary; tests CSV escaping / xlsx string cells end-to-end |
| 5.3 | PENDING draft review on merged PR | [#47](https://github.com/stefanpenner/gh-audit-test-fixtures/pull/47) | no | drafts are not audit events; must be filtered, must not fail the batch |
| 5.4 | One commit in two merged PRs | [#48](https://github.com/stefanpenner/gh-audit-test-fixtures/pull/48), [#49](https://github.com/stefanpenner/gh-audit-test-fixtures/pull/49) | no | `pr_count=2`; surfaces on the Multiple PRs sheet |

---

## Running validation locally

```bash
GITHUB_TOKEN=$(gh auth token) ./scripts/test-fixtures.sh
```

Output groups assertions by series, shows clickable commit links, and reports per-field mismatches on failure.

## Adding a new scenario

1. Create the fixture (branch, PR, reviews, merge) in this repo
2. Add an entry to `scenarios.json` with the commit SHA and expected field values
3. Run the validation script to confirm it passes
4. The CI job in gh-audit runs this on every push to main
