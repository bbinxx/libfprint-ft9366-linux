# Git workflow

This repository has independent history. Never fork GitHub or import reference history.
The initial root commit bootstraps dev; subsequent work uses short-lived branches.

```sh
git status --short --branch
git switch dev
git switch -c feature/frame-decoder
# edit and test; stage explicit paths only
git diff --cached --check
git diff --cached
# commit only reviewed scope
git switch dev
git merge --no-ff feature/frame-decoder
```

Prefixes: feature/, fix/, docs/, chore/. Keep dev permanent. Use meaningful messages:
feat:, fix:, docs:, build:, test:, chore:. Review provenance, test results and privacy
before merging. Never reset/clean user work or rewrite shared history. Keep task
branches until their merge is verified; no automatic deletion required.

No remote creation, push, GitHub publishing or AUR submission without approval.
Use annotated version tags only for validated releases; there is no release tag yet.
Normally use actual commit dates. The owner explicitly requested August 23, 2026
for the foundation commit; author and committer timestamps are intentionally backdated.
Evidence collected November 13–14, 2026 retains those actual dates.
