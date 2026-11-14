# Versioning policy

VERSION tracks this project's development version: 0.1.0-dev, unreleased.
Use MAJOR.MINOR.PATCH for published project releases. Before 1.0, minor versions
may change experimental interfaces; patches fix compatible behavior. Document every
release's supported hardware and tested libfprint versions. A version does not imply
hardware reliability. Keep immutable tags such as v0.1.0 with a release manifest.

Arch pkgver/pkgrel describe packaged artifacts separately. The current Track A
baseline uses `1.94.100.ft9366.20261021-1`: upstream libfprint version, FT9366
marker, and integration snapshot date. Source commits and checksums are
authoritative in the PKGBUILD and upstream notice. Do not label third-party
driver code as original. Increment pkgrel for packaging-only rebuilds. This
experimental package version is not a stable release and does not imply
successful enrollment or verification. Record template format versions
separately; no Track B template format exists yet.
