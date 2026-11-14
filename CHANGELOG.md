# Change history

## 0.1.0-dev — unreleased

Actual work dates: November 13–14, 2026. Foundation commit timestamps intentionally
set to August 23, 2026 at the owner's request; this is not the research date.

- Record initial environment, USB and source compatibility evidence.
- Add original offline frame decoding with synthetic bounds/byte-order tests.
- Add bounded fragmented capture assembly and cancellation/error states; no USB I/O.
- Document development, safety gates, attribution, branching and versioning.
- Add backend-neutral asynchronous USB transfer core with bounds and cancellation tests.
- Add FpiUsbTransfer adapter with per-transfer cancellation and simulated tests against pinned headers.
- Add interface lifecycle management with cancel/drain close and simulated ownership/error tests.
- Record owner-reported successful physical USB open, interface claim/release and close.
- Add explicit single-read hardware probe with a one-second timeout and no payload logging.
- Prepare isolated reference-sequence capture experiment and exact operation manifest.
- Record owner-reported approved reference initialization, touch detection and first 10240-byte frame acquisition; no image retained.
- Add pinned, attributed Track A Arch packaging; 130 upstream tests pass.
- Record owner-approved Pacman installation, completed enrollment and one successful verification.
- Original Track B integration and repeated hardware reliability checks remain incomplete.
