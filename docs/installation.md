# Installation and rollback

**The owner installed and tested this Track A package on November 14, 2026.**
See [hardware validation](hardware-validation.md) for the observed result and
remaining limits. For a new installation, it replaces the system `libfprint`
package. Installation requires a separate explicit approval after reviewing the
Pacman transaction. It also installs a narrowly matched udev rule setting
`power/control=on` for `2808:a658`, so autosuspend remains disabled for this
reader while the package is installed. No PAM file is changed and no service,
session, or systemd target is restarted by these instructions.

## Preflight

From the project root, inspect the artifacts and current packages:

```sh
pacman -Q libfprint fprintd
pacman -Qip artifacts/arch/libfprint-ft9366-arch-1.94.100.ft9366.20261021-1-x86_64.pkg.tar.zst
pacman -Qip artifacts/rollback/libfprint-1.94.100-2-x86_64.pkg.tar.zst
gpgv --keyring /etc/pacman.d/gnupg/pubring.gpg \
  artifacts/rollback/libfprint-1.94.100-2-x86_64.pkg.tar.zst.sig \
  artifacts/rollback/libfprint-1.94.100-2-x86_64.pkg.tar.zst
sha256sum artifacts/arch/*.pkg.tar.zst artifacts/rollback/*
```

The verified rollback archive matches the original `libfprint 1.94.100-2`; its
dependencies were present at build time. Confirm that remains true on the day
of installation. The experimental package depends on the same runtime library
packages, provides the `libfprint-2.so=2-64` dependency required by installed
`fprintd`, and conflicts with `libfprint`.

Review Pacman's proposed transaction first:

```sh
sudo pacman -U --print \
  artifacts/arch/libfprint-ft9366-arch-1.94.100.ft9366.20261021-1-x86_64.pkg.tar.zst
```

Stop if Pacman proposes removing anything besides replacing `libfprint`, or if
it proposes removing `fprintd` or another desktop/authentication component.
The expected package metadata has no install script. Proceed only after
reviewing the transaction and explicitly approving installation.

## Install after approval

```sh
sudo pacman -U \
  artifacts/arch/libfprint-ft9366-arch-1.94.100.ft9366.20261021-1-x86_64.pkg.tar.zst
```

Keep the current session and password authentication available. Do not edit
PAM. The install changes only Pacman-managed package files and the package's
device-specific udev rule. Do not manually copy libraries into `/usr/lib`.

## Test discovery, enrollment, and verification

First check the USB presence and query the library:

```sh
lsusb -d 2808:a658
fprint-list-supported-devices
```

Then check fprintd and attempt enrollment in the user's terminal:

```sh
fprintd-enroll -f right-index-finger "$USER"
fprintd-verify -f right-index-finger "$USER"
```

Enrollment/verification require touching the sensor and may store a biometric
template through fprintd. Do not save or publish raw images, templates, or
debug dumps. Record only commands, non-biometric status/errors, and package
versions. If discovery or enrollment fails, stop and collect relevant
non-biometric journal output; do not guess protocol writes or change PAM.

The earlier owner-run reference capture transferred a 10240-byte frame only;
it did not inspect image quality or validate matching. Thus successful package
installation or device discovery is not itself a success claim for fingerprint
authentication.

## Roll back

Use the preserved official package with Pacman:

```sh
sudo pacman -U artifacts/rollback/libfprint-1.94.100-2-x86_64.pkg.tar.zst
```

Pacman will handle the conflict with the experimental package and restore the
official library. Check the transaction prompt before confirming. Verify the
result with:

```sh
pacman -Q libfprint fprintd
```

Do not reboot or restart SDDM for this rollback. The replacement package's udev
rule is removed as part of package removal. If the original behavior does not
return, inspect package and udev state before making any additional change.
