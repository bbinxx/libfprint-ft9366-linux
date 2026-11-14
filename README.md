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
