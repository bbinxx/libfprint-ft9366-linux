# libfprint-ft9366-linux

Independent FT9366 fingerprint driver development for Linux.

Experimental project targeting USB `2808:a658`. The repository contains original
Track B protocol/decoder research code and a separately attributed Track A Arch
package built from pinned upstream FT9366 sources. Track A is an installable
compatibility baseline; it is not an independently implemented driver. The
package builds and passes the upstream test suite, and its library discovers
this device. The owner completed enrollment and one successful verification
with the installed Track A package on November 14, 2026. Repeated verification,
wrong-finger rejection, and suspend/resume reliability remain unverified.
See the [hardware validation report](docs/hardware-validation.md).

Hardware milestone: the owner reports one successful finger-triggered 10240-byte
frame acquisition using the separately approved reference-sequence experiment.
No image was saved or inspected; this does not validate the original driver or
authentication. See the [hardware result](docs/reference-capture-experiment.md).

Track A means independently maintained packaging of pinned upstream driver code.
Track B means an original implementation informed by documented evidence. These
tracks must remain clearly attributed. No upstream Git history is imported.

Run original-code synthetic tests with `make test`. Build the Track A package from
the repository root with `python3 scripts/build_arch_package.py`; it pins and
checks upstream sources, runs the libfprint test suite, and saves the package in
`artifacts/arch/`. The script never installs it. See [build and package status](docs/packaging.md)
and [installation and rollback](docs/installation.md).

## Distribution support

The project targets Linux. Only Arch Linux has been tested so far: USB
`2808:a658`, libfprint 1.94.100, and fprintd 1.94.5. Other distributions require
their own packaging and compatibility validation. The Arch package cannot be
installed directly on Debian, Ubuntu, Fedora, or other package formats.

Project branding is `libfprint-ft9366-linux`. The tested Arch binary retains
`libfprint-ft9366-arch` as its distro-specific package name. Existing local
checkout paths can remain unchanged. See the [Arch release notes](docs/releases/v0.1.0-arch.1.md).

## Installation and usage

### 1. Check your hardware

```sh
lsusb -d 2808:a658
```

The tested USB ID is **2808:a658**. Other IDs have not been validated. Only Arch
Linux x86_64 currently has an installable package and real enrollment/verification
evidence. Ubuntu, Debian and Fedora have source-build CI jobs; DEB/RPM installers
are not provided. Do not install the Arch package on those distributions.

### 2. Get the project

```sh
git clone https://github.com/bbinxx/libfprint-ft9366-linux.git
cd libfprint-ft9366-linux
git switch main
```

Existing checkout: inspect `git status` before updating; preserve your changes.
The repository name ends in `linux`, while the Arch package is named
`libfprint-ft9366-arch`.

### 3. Prepare Arch dependencies and rollback

Inspect the current package versions before replacing libfprint:

```sh
pacman -Q libfprint fprintd
```

Before installation, preserve the official libfprint package matching your
system and its detached signature from `/var/cache/pacman/pkg/`. Keep them
outside temporary build directories. If the matching archive is missing,
obtain and verify an appropriate official rollback package before continuing.
The development machine used libfprint 1.94.100-2; do not assume that version
is suitable for your system. See [rollback preparation](docs/installation.md).

Verify the saved archive, using its actual paths:

```sh
gpgv --keyring /etc/pacman.d/gnupg/pubring.gpg \
  /path/to/official-libfprint.pkg.tar.zst.sig \
  /path/to/official-libfprint.pkg.tar.zst
pacman -Qip /path/to/official-libfprint.pkg.tar.zst
```

Install runtime utilities through Pacman after reviewing its transaction. Arch
requires a complete system upgrade rather than a partial package upgrade:

```sh
sudo pacman -Syu --needed fprintd usbutils
```

Recheck the installed version and rollback compatibility after any upgrade.
Keep password authentication and your current session available. This guide
changes no PAM configuration.

### 4A. Install a reviewed release package

Open [GitHub Releases](https://github.com/bbinxx/libfprint-ft9366-linux/releases)
and download the Arch x86_64 `.pkg.tar.zst` and `SHA256SUMS` from the same
reviewed version. Drafts are visible only to maintainers; a tag creates a draft
only after its build jobs succeed. If no release is available, use the source
build below.

From the download directory, verify the package entry in SHA256SUMS. If you
also downloaded the source archives, the same command verifies those too:

```sh
sha256sum --check --ignore-missing SHA256SUMS
```

The command ignores assets you did not download. The package itself must
report `OK`; stop on any checksum mismatch or if the package was not checked.
Assets are unsigned; a checksum is not an independent publisher signature.

Inspect and install the exact reviewed file, replacing the example version if
needed:

```sh
pacman -Qip ./libfprint-ft9366-arch-1.94.100.ft9366.20261021-1-x86_64.pkg.tar.zst
sudo pacman -U ./libfprint-ft9366-arch-1.94.100.ft9366.20261021-1-x86_64.pkg.tar.zst
```

Pacman should offer to remove the conflicting official `libfprint` and install
`libfprint-ft9366-arch`, keeping `fprintd`. Stop if unrelated desktop or
authentication packages would be removed. The package installs a USB-ID-specific
udev rule setting `power/control=on` to disable autosuspend for this reader.
A newly built CI package needs hardware validation even when an earlier build
has worked.

### 4B. Build the Arch package yourself

From the repository root, install the build/check dependencies declared by the
current [PKGBUILD](packaging/arch/PKGBUILD):

```sh
sudo pacman -Syu --needed base-devel git python namcap meson \
  glib2-devel gobject-introspection gtk-doc python-cairo python-gobject \
  systemd cairo umockdev libgudev libgusb openssl pixman
python3 scripts/build_arch_package.py
```

Run the build as your normal user. It checks pinned sources, compiles libfprint,
runs the upstream tests and package lint, and prints the package path and SHA256.
It never installs the replacement. Output is saved under `artifacts/arch/`;
repeated builds may use a subdirectory to preserve earlier artifacts. Use the
exact printed package path with `pacman -Qip`, then `sudo pacman -U` as above.
Recheck rollback compatibility after installing build dependencies or upgrading.

### 5. Discover the device and enroll

Run these commands in your logged-in desktop terminal. Polkit may request local
authorization; an agent/background shell can be denied enrollment permission.

```sh
pacman -Q libfprint-ft9366-arch fprintd
lsusb -d 2808:a658
fprintd-list "$USER"
fprintd-enroll -f right-index-finger "$USER"
```

Touch and lift the right index finger as prompted. `enroll-stage-passed` means
progress; wait for **`enroll-completed`** before trying verification.
`enroll-finger-not-centered` means reposition your finger and retry. Enrollment
creates a private template managed by fprintd; do not publish it or raw images.

### 6. Verify the enrolled finger

```sh
fprintd-verify -f right-index-finger "$USER"
```

**`verify-match`** confirms that attempt matched. Check repeated attempts with
the enrolled finger and rejection with an unenrolled finger. One successful
match does not establish reliable or secure authentication. Leave sudo, SDDM,
gtklock and other PAM integrations for a separately reviewed milestone.

### 7. Troubleshoot or restore official libfprint

| Result | Next step |
| --- | --- |
| `NoEnrolledPrints` | Complete enrollment, then check `fprintd-list` again. |
| `NoSuchDevice` | Check USB ID and installed package; inspect service errors. |
| Polkit denial | Enroll from the active desktop terminal and authorize locally. |
| Timeout / initialization failure | Record the error and versions; avoid speculative USB writes. |
| fprintd exits after inactivity | Normal D-Bus activation behavior; not by itself a daemon fault. |

Read recent service errors without enabling raw biometric logging:

```sh
journalctl -b -u fprintd --no-pager -n 80
```

To roll back, use the verified official package saved before installation:

```sh
sudo pacman -U /path/to/official-libfprint.pkg.tar.zst
pacman -Q libfprint fprintd
```

Review the conflict/replacement prompt. Pacman restores the official library
and removes the custom package's udev rule. Do not copy libraries manually into
`/usr/lib`, restart SDDM, or reboot just to install/restore this package.
See [troubleshooting](docs/troubleshooting.md) and
[installation and rollback details](docs/installation.md).

### Other distributions and release automation

The source release includes pinned libfprint/reference sources, integration
patches and attribution. Native package integration, ABI checks and rollback
must be developed for each additional distribution before system installation.
CI compilation alone does not establish hardware support. Do not run Arch's
Pacman commands on Ubuntu, Debian or Fedora, and do not manually install a
source-built replacement over system-managed libraries.

[Release automation](docs/release-workflow.md) builds the Arch package and checks
Ubuntu, Debian and Fedora source builds. Successful version-tag workflows create
**draft prereleases** with corresponding sources and checksums for review. The
[user guide](docs/user-guide.md) and [packaging guide](docs/packaging.md) provide
additional development details.

## Guides

- [First hardware connection](docs/hardware-connection.md), [reference capture proposal](docs/reference-capture-experiment.md)
- [Phase 1 evidence](docs/phase-1-report.md)
- [Arch package validation](docs/arch-build-report.md)
- [Architecture](docs/architecture.md), [hardware](docs/hardware.md), [protocol](docs/usb-protocol.md)
- [Transport contract](docs/transport.md), [libfprint adapter](docs/libfprint-usb-adapter.md), [device lifecycle](docs/device-lifecycle.md)
- [Development](docs/development.md), [testing](docs/testing.md), [packaging](docs/packaging.md), [installation](docs/installation.md)
- [Compatibility](docs/compatibility.md), [troubleshooting](docs/troubleshooting.md)
- [Windows analysis](docs/windows-driver-analysis.md)
- [Git workflow](docs/git-workflow.md), [versioning](docs/versioning.md)
- [Roadmap](docs/roadmap.md), [licensing](docs/licensing.md), [contributing](CONTRIBUTING.md)
- [Agent instructions](AGENTS.md), [change history](CHANGELOG.md)

The package replaces Arch's `libfprint` and adds a narrowly scoped udev rule that
keeps autosuspend disabled for this USB ID. Review the exact transaction and rollback
instructions before installing; installation still requires owner approval. Keep
password authentication available and PAM unchanged until reliable enrollment and
verification. Do not commit biometric data.

License selection for original work is pending; no project-wide open-source license
has yet been granted. See licensing guidance and NOTICE before redistribution.
