# Arch packaging plan

No PKGBUILD or installable package exists yet. Start from the verified official
1.94.100-2 recipe recorded in the Phase 1 report, preserving upstream attribution.
Pin libfprint and any Track A source by immutable revision and verify integrity.
Download only during source retrieval, never build(). Stage installs into pkgdir.

After dependency approval, use makepkg -s (without -i), inspect Meson output,
run upstream tests, inspect package contents/dependencies/SONAME and lint with namcap.
Generate .SRCINFO with makepkg --printsrcinfo only from the reviewed recipe.
Validate in a clean Arch chroot when available. Confirm provides/conflicts and
fprintd compatibility from the resulting artifact rather than assuming them.

Before installation: verify official rollback package signature and dependencies,
inspect the proposed Pacman transaction and obtain approval. Install via Pacman only.
