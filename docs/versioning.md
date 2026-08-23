# Versioning policy

VERSION tracks this project's development version: 0.1.0-dev, unreleased.
Use MAJOR.MINOR.PATCH for published project releases. Before 1.0, minor versions
may change experimental interfaces; patches fix compatible behavior. Document every
release's supported hardware and tested libfprint versions. A version does not imply
hardware reliability. Keep immutable tags such as v0.1.0 with a release manifest.

Arch pkgver/pkgrel describe the packaged artifacts separately. Track A packaging
must expose the pinned libfprint version, reference revision and integration revision;
do not label third-party code as this project's original driver. pkgrel increments
for packaging-only rebuilds. Final package naming/version ordering awaits a validated
recipe. Record template format versions separately; never silently reinterpret stored
biometric templates. No template format exists in this project yet.
