# Linux build and release automation

.github/workflows/linux-release.yml builds on pull requests, dev/main pushes,
manual dispatch and version tags. Source builds target Ubuntu 24.04, Debian 12,
and Fedora 44 containers. An Arch container builds the native package, executes
the recipe's upstream checks and runs namcap. Container builds do not attach
hardware or validate fingerprint matching. These jobs are added for validation;
no successful GitHub run is claimed until Actions reports one.

There is no universal binary installer for every Linux distribution. Current
release assets are an Arch x86_64 package, a corresponding source bundle, the
project source archive, and SHA256SUMS. DEB, RPM, ARM and other package targets
require separate packaging work. Container tags select distro versions but are
not digest-pinned snapshots; dependency versions can change between builds.
Official GitHub Actions are pinned to immutable commits.

To prepare a release, merge reviewed changes into main and choose an immutable
version tag. The first proposed Arch baseline is v0.1.0-arch.1. After explicitly
reviewing the tag target and authorization to push, a maintainer can run:

```sh
git tag -a v0.1.0-arch.1 main -m 'Experimental Arch baseline'
git push origin v0.1.0-arch.1
```

All required build jobs must pass before the release job runs. That job uses
GitHub's built-in token with contents:write only in the release job and creates
a draft prerelease. It will update an existing draft but refuse to overwrite a
published release. Manual workflow dispatch runs builds without creating a
release. Review source licenses, attribution, checksums, test results and asset
contents before publishing the draft through GitHub. Original-code licensing
is still undecided; do not publish repository source until it is resolved.

The source preparation script downloads immutable archives, verifies both
upstream and local PKGBUILD BLAKE2 hashes, applies the reviewed patches, and
copies the pinned reference driver into libfprint. It does not install anything.
Build logs on GitHub are evidence for compilation/tests only. A newly built
release binary is different from the earlier locally hardware-tested artifact;
retain that distinction in release notes. No PAM, device writes or proprietary
components are part of CI.

References: GitHub's container documentation at
https://docs.github.com/en/actions/tutorials/use-containerized-services/create-a-docker-container-action
and the official action repositories https://github.com/actions/checkout,
https://github.com/actions/upload-artifact and https://github.com/actions/download-artifact.

## Local validation of this workflow change

On November 14, 2026, pinned-source preparation and checksum checks passed,
as did local compilation with drivers=ft9366 and introspection enabled. The
selected upstream suite reported 4 passed, 34 skipped, 0 failed; excluded driver
tests account for the skips. The full Arch package had previously passed 130
tests. YAML parsing and bash -n checks passed for every run block. Docker
daemon access was unavailable locally, so the distro containers and GitHub
release upload have not been executed here. Their first Actions run remains
required evidence.
