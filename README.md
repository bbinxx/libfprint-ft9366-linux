# Independent FT9366 development for Arch Linux

Experimental project targeting USB `2808:a658`. Original Track B code currently
provides a frame decoder, capture receive state machine, and backend-neutral
asynchronous USB transport core, libfprint USB adapter, and interface lifecycle
helper tested offline. USB initialization, libfprint integration,
enrollment and verification are not implemented or validated here.

Hardware milestone: the owner reports one successful finger-triggered 10240-byte
frame acquisition using the separately approved reference-sequence experiment.
No image was saved or inspected; this does not validate the original driver or
authentication. See the [hardware result](docs/reference-capture-experiment.md).

Track A means independently maintained packaging of pinned upstream driver code.
Track B means an original implementation informed by documented evidence. These
tracks must remain clearly attributed. No upstream Git history is imported.

Run synthetic tests with `make test`. Builds default to `/tmp`; no USB access or
installation occurs. Requirements: a C11 compiler, make, Python 3, pkg-config and libusb development files.

## Guides

- [First hardware connection](docs/hardware-connection.md), [reference capture proposal](docs/reference-capture-experiment.md)
- [Phase 1 evidence](docs/phase-1-report.md)
- [Architecture](docs/architecture.md), [hardware](docs/hardware.md), [protocol](docs/usb-protocol.md)
- [Transport contract](docs/transport.md), [libfprint adapter](docs/libfprint-usb-adapter.md), [device lifecycle](docs/device-lifecycle.md)
- [Development](docs/development.md), [testing](docs/testing.md), [packaging](docs/packaging.md)
- [Compatibility](docs/compatibility.md), [troubleshooting](docs/troubleshooting.md)
- [Windows analysis](docs/windows-driver-analysis.md)
- [Git workflow](docs/git-workflow.md), [versioning](docs/versioning.md)
- [Roadmap](docs/roadmap.md), [licensing](docs/licensing.md), [contributing](CONTRIBUTING.md)
- [Agent instructions](AGENTS.md), [change history](CHANGELOG.md)

Do not install an experimental replacement without a verified rollback package
and explicit approval. Keep password authentication available and PAM unchanged
until reliable hardware enrollment and verification. Do not commit biometric data.

License selection for original work is pending; no project-wide open-source license
has yet been granted. See licensing guidance and NOTICE before redistribution.
