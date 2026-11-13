# Reviewed reference-based capture experiment

Status: owner reports successful hardware execution on November 14, 2026, after
explicit approval. This is a Track A-style reference baseline, separate from the
original Track B libfprint driver. This
experiment uses upstream replay data without claiming those sequences as original.

## Provenance and build

Reference: ilyamiro/ft9366-fingerprint, revision
5420cd9b7124e5a70deaae4bffedd969ae4ef91c, LGPL-2.1-or-later.
The existing external checkout supplies ft9366_script.h at compile time. The
build helper checks the pinned commit and refuses tracked header/license changes.
No upstream source file or Git history is imported into this repository.
A license copy and attribution notice accompany the executable in /tmp.
Original runner and bounded host-record parser live under tools/.

```sh
make reference-capture-probe REFERENCE_SOURCE=/home/bbin/fingerprint-driver-review/ft9366-fingerprint
make reference-capture-test REFERENCE_SOURCE=/home/bbin/fingerprint-driver-review/ft9366-fingerprint
```

The first command only builds and writes an exact command manifest:
/tmp/ft9366-capture-reference-plan.txt. Read it before approving hardware execution.
Default executable behavior and --plan perform no USB operation. --simulate always
builds a distinct .sim executable that prints SIMULATED USB BACKEND; it cannot
replace the hardware executable. The simulation flag never exists in production.

## Approved hardware scope

1. Keep finger OFF sensor; owner presses Enter before any USB operation.
2. Find exactly one 2808:a658, validate the observed interface layout, open handle
   and refuse to detach any active kernel driver.
3. Perform one USB reset, claim interface 0, and wait 500 ms as in the reference.
4. Replay the pinned init: 223 OUT writes and 73 IN reads, including four
   10240-byte frames. All replies/frames are cleared; none are saved.
5. Prompt owner to place/hold a finger. Poll the reference idle script (46 OUT,
   18 IN per iteration), at most 40 cycles and a checked 10-second poll deadline.
6. If the reference's 0x0002 flag is present, run scan once and discard that first
   frame. Recheck the flag with one idle script, wait 60 ms and run scan again.
   Each scan has 46 OUT and 13 IN operations including one 10240-byte frame.
7. Report transfer completion only, erase the reply buffer and release/close.

Worst-case operation caps: 2201 OUT and 837 IN requests. Not all paths reach these
caps. No retry after a failed transfer, no firmware file loading, no persistent
power/udev changes, no package installation, no fprintd/PAM changes. Individual
transfers use at most 3000 ms and replay observes a checked 60-second software
deadline after reset/claim; OS reset/claim calls themselves are not preempted by
that deadline. SIGINT/SIGTERM are checked between replay operations.

The register meanings and effects of many writes remain unresolved. This is an
explicitly reviewed replay experiment, not a claim that all writes are proven safe
or necessary. Do not run it without approval for this exact expanded scope.

## Privacy and interpretation

The runner writes no biometric files, payload logs or payload hashes. Data exists
only in a fixed 10240-byte buffer cleared using volatile stores after each transfer
and on failure. It does not process, render or match the fingerprint.
A finger flag plus a full-size frame is evidence of acquisition behavior, not
proof of useful ridge quality, successful enrollment or reliable verification.
The experiment does not prove the original driver works.

## Offline validation

Parser tests reject truncated headers/payloads, unknown operations, excessive
lengths/delays and offset errors. The full executable has simulated success,
short-response, disconnect and no-finger paths with cleanup assertions. Test
symbols are linked only into the separately named .sim executable. The real
executable links the installed libusb/GLib. No Windows driver is executed.
## First owner-reported hardware result

The owner explicitly approved the reset/OUT scope, then ran:

```sh
sudo /tmp/ft9366-capture-reference --capture
```

Executable SHA256 recorded before this run:
1046f4000a9922cf89a787b03a3c9cbf5ab228ee679b148a2bfdf9593f59f035.
This is owner-supplied terminal evidence, not an agent-executed privileged run.

```text
Reference initialization transfers completed; four calibration-sized frames discarded.
PLACE AND HOLD your finger flat on the sensor now (up to 10 seconds).
Reference finger-present flag observed; requesting frames.
First touch frame received and discarded, following reference settling behavior.
Finger-triggered 10240-byte sensor frame received in memory and cleared.
This confirms frame transfer only; fingerprint ridge quality, enrollment and matching are unverified.
USB resources closed; no biometric payload saved.
```

Observed scope: the reference initialization, finger-status polling, first-frame
discard and second frame-sized read all completed without a reported transfer
failure, followed by resource cleanup. Numeric process exit status was not supplied.
No image was retained or inspected, so visual ridge quality, orientation, contrast,
and the claimed 16-bit encoding are not independently verified by this transcript.
It is one successful acquisition, not a repeatability or authentication test.
The original libfprint driver and fprintd path remain unvalidated on hardware.

