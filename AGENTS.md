# Instructions for contributors and agents

## Scope and evidence

Read README.md, docs/phase-1-report.md and the relevant guide before editing.
Inspect Git status first; preserve staged, unstaged and untracked user work.
Work incrementally. Keep observed hardware behavior, source evidence, hypotheses
and unknowns separate. Never claim compilation or hardware success without results.
Use actual observation dates even if a commit timestamp is intentionally backdated.

## Independence and implementation

Maintain independent history. Never fork or import another project's Git history.
Track A packages upstream code with pinned revisions and attribution. Track B is
original implementation. Record provenance for protocol knowledge and borrowed code.
Do not silently switch tracks. Prefer bounded C parsers, explicit ownership,
asynchronous libfprint USB helpers, cancellation, deadlines and cleanup.
Do not invent initialization commands or add unsupported USB IDs.

## Safety gates

Explicit approval is required before installing packages, replacing libfprint,
experimental USB operations, persistent power changes, PAM changes, remote creation
or pushing. Prepare verified rollback before experimental package installation.
Never manually replace /usr/lib, restart SDDM, force logout, reboot, disable security
mechanisms, run unknown scripts or execute Windows drivers. No speculative USB writes.
Do not stop unrelated applications or active sessions.

## Privacy and filesystems

No raw images, templates, capture payloads or private logs in Git. Use synthetic
fixtures. Ignore rules are not access control. This workspace's fuseblk permissions
do not enforce chmod; store private material on a protected native filesystem.
Keep reference checkouts and build artifacts outside the project history.

## Workflow and validation

Use dev as permanent integration branch; short-lived feature/, fix/, docs/, chore/
branches after the initial root commit. Follow docs/git-workflow.md. Do not reset,
clean, force-push, or remove user changes. Do not spawn agents unless requested.
Run make test for decoder changes and record exact checks and limitations.
Inspect staged diff before committing; never stage indiscriminately.
Do not change licensing or publish releases without owner approval.
