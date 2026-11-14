# Using libfprint-ft9366-linux

This project develops FT9366 support for Linux. Its currently tested installation
is the Track A Arch package, which integrates the attributed reference driver.
Original Track B development continues separately. Only USB `2808:a658` has local
hardware evidence: one completed enrollment and one successful verification.

## Get the repository

```sh
git clone https://github.com/bbinxx/libfprint-ft9366-linux.git
cd libfprint-ft9366-linux
git switch main
lsusb -d 2808:a658
```

A different USB ID is not supported merely because its product name is similar.
Read README.md and docs/compatibility.md before installing anything.

## Arch Linux installation

Use the Arch x86_64 package from a reviewed GitHub release, or build it locally.
Draft releases and CI artifacts are experimental. A CI-built binary has not itself
been hardware-tested simply because an earlier local build worked.

For a source build, install the dependencies listed in packaging/arch/PKGBUILD,
plus base-devel, Python and namcap, through Pacman after reviewing the changes.
Then run as your normal user:

```sh
python3 scripts/build_arch_package.py
```

Read the printed output path: repeated builds may preserve earlier packages and
place the new result in a separate directory below artifacts/arch/. The script
builds and tests the package without installing it. Never run makepkg as root.

Before installing, preserve a matching official libfprint rollback package and
its detached signature from the Pacman cache, verify that signature, and review
the proposed replacement transaction. Follow docs/installation.md for the exact
preflight and rollback procedure; the listed 1.94.100-2 rollback is specific to
the development machine and may not suit a different system.

Check a downloaded release's SHA256SUMS from the release directory:

```sh
sha256sum -c SHA256SUMS
```

Checksums detect changed downloads; unsigned assets do not provide an independent
publisher signature. Install only the exact package you reviewed:

```sh
sudo pacman -U /path/to/reviewed-package.pkg.tar.zst
```

This replaces libfprint and installs a narrowly matched udev rule disabling
USB autosuspend for `2808:a658`. It does not configure PAM. Keep password
access available and stop if Pacman proposes removing unrelated packages.

## Enroll and verify

Run from your logged-in desktop terminal so Polkit can authorize enrollment:

```sh
fprintd-list "$USER"
fprintd-enroll -f right-index-finger "$USER"
```

Follow the sensor prompts. Touch and lift between stages. Intermediate
`enroll-stage-passed` results are progress; wait for `enroll-completed`.
`enroll-finger-not-centered` requests another attempt. If the command exits
without completion, do not assume a template was saved.

After completion:

```sh
fprintd-verify -f right-index-finger "$USER"
```

`verify-match` confirms that attempt matched. Check several independent attempts
with the enrolled finger and confirm rejection with an unenrolled finger before
considering any authentication integration. Do not alter PAM from this guide.

## Problems and recovery

- `NoEnrolledPrints`: finish enrollment first, then check fprintd-list again.
- Polkit denial: run from the active desktop session and authorize locally.
  Do not weaken policy or send passwords through chat/logs.
- `NoSuchDevice`: check USB ID, installed package and fprintd journal before
  changing USB settings. An idle fprintd daemon exiting normally is expected.
- Timeout or failed initialization: stop, record the non-biometric error and
  package versions, and consult docs/troubleshooting.md. Avoid random USB writes.
- Package failure: restore the verified official libfprint package with
  `sudo pacman -U /path/to/verified-official-package.pkg.tar.zst`, reviewing the
  transaction. Do not manually replace files under /usr/lib.

Enrollment stores biometric templates through fprintd. Keep templates and raw
sensor images private; never attach them to issues or commit them to Git.

## Other Linux distributions

Do not install the Arch package on Debian, Ubuntu or Fedora. The release source
bundle includes the pinned upstream sources, applied integration, license texts
and Arch recipe for provenance. The CI workflow checks source compilation on
Ubuntu, Debian and Fedora; these jobs do not create supported DEB/RPM installers
or validate real sensors. Native packaging, dependency/ABI compatibility and
rollback must be reviewed on each target distribution before system installation.
See docs/release-workflow.md for current automation scope and limitations.
