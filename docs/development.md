# Development

Current build is an offline C module: `make test`. No Meson or libfprint build
is needed for it. Sources live in driver/, synthetic tests in tests/.
Build output defaults to /tmp/libfprint-ft9366-arch-build; override BUILD_DIR on a
native Linux filesystem. Never store biometric material on this fuseblk workspace.

For libfprint work first resolve missing tools from the Phase 1 inventory with
approval. Fetch pinned sources outside this repository, verify signatures and hashes,
then apply small reviewed Meson/driver registration patches in a disposable tree.
Build without installation and run upstream tests. Document exact commands, compiler,
source revisions, configuration and results. Keep package work separate from original
protocol code; no floating revisions for releases.
