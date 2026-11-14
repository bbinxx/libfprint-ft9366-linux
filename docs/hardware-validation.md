# Track A hardware validation

Observation date: November 14, 2026. Evidence is owner-supplied terminal output
and local package/service inspection. No biometric payload is included here.

## Installed baseline

USB VID:PID: `2808:a658`, FocalTech FT9366 through the reported RTS5811 bridge.
The owner approved and performed a Pacman transaction replacing official
`libfprint 1.94.100-2` with `libfprint-ft9366-arch
1.94.100.ft9366.20261021-1`. `fprintd 1.94.5-2` remained installed.
Pacman's hardware database update and device-manager reload hooks completed.
This package integrates pinned upstream reference code (Track A); the original
Track B implementation remains incomplete. See [package provenance](packaging.md).

## Observed results

1. `fprintd-list` exposed one device named
   `FocalTech FT9366 (RTS5811 bridge)`.
2. An agent-shell enrollment attempt timed out. The service journal recorded
   Polkit denial of `EnrollStart`; this did not establish a sensor failure.
3. Initial owner attempts reached intermediate enrollment stages; verification
   returned `NoEnrolledPrints` before any completed enrollment.
4. Enrollment from the owner's terminal reached `enroll-completed` after
   multiple `enroll-stage-passed` statuses and two
   `enroll-finger-not-centered` retries.
5. A subsequent `fprintd-verify -f right-index-finger "$USER"` listed the stored
   right-index finger and finished with `verify-match (done)`.

This establishes one completed enrollment and one successful verification on
this machine. It also confirms the saved template was available to a subsequent
client invocation. It does not establish authentication reliability or security.

## Remaining validation

- Repeat verification with the enrolled finger across separate invocations.
- Confirm rejection with an unenrolled finger without overwriting enrollment.
- Check cancellation, timeout recovery, suspend/resume, and persistence across
  service restart or reboot when separately authorized.
- Perform a rollback installation test when appropriate; the official archive
  and signature have been preserved and verified, but rollback has not been run.
- Keep PAM integration pending until repeatable verification and rejection are
  established. PAM configuration has not been changed.

Templates remain private host data managed by fprintd. Do not copy them into the
repository or publish raw sensor frames. No new hardware operations were run
when documenting these results.
