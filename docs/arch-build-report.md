# Arch build and package validation

Observed 2026-11-14 on Arch Linux x86_64. This report concerns the Track A
package built from pinned sources, not the repository's original Track B code.

## Inputs and build

- Arch's official recipe baseline: `libfprint 1.94.100-2`, packaging commit
  `4754ff229c0e4cb566f33e99e962d702bfc52970`.
- libfprint source commit: `80a4b5ec612892c5c056c48dddaa561452cf37ec`.
- FT9366 reference commit: `5420cd9b7124e5a70deaae4bffedd969ae4ef91c`.
- During the build, installed `libfprint 1.94.100-2` and `fprintd 1.94.5-2`
  were unchanged. The owner subsequently installed the experimental package;
  see [hardware validation](hardware-validation.md).
- Meson 1.12.1, GCC 16.2.1, gobject-introspection 1.86.0,
  gtk-doc 1.37.0, umockdev 0.19.9.
- `SOURCE_DATE_EPOCH=1792590607`; native host build, not a clean chroot.
- Both downloaded archives and local patches passed the PKGBUILD BLAKE2 checks.
- Final serial Meson suite: **130 passed, 0 failed**. The suite is upstream's
  test set and does not test FT9366 hardware behavior.
- `namcap` on recipe and package: no findings.
- Package metadata: x86_64; provides `libfprint=1.94.100` and
  `libfprint-2.so=2-64`; conflicts with `libfprint`; same seven runtime
  dependencies as official package; no install script.
- `fprintd` remains installed and its SONAME dependency is satisfied by package
  metadata. Public exported symbol names match the installed library.
- Built library discovery identified driver `ft9366`, name
  `FocalTech FT9366 (RTS5811 bridge)`. This was a non-installed, non-opening
  discovery call, not a hardware enrollment test.
- Device-specific udev rule passed `udevadm verify`; package includes the
  reference upstream and libfprint license notices.

The build helper stores the package SHA256 and test counts in
`artifacts/arch/build-*.json`. The final package SHA256 at report creation is
`2cce216ec5f51c8a3d6fd1fa57536b792985ee74d4830bfe9ea793cf891935e5`.
Package artifact:
`artifacts/arch/libfprint-ft9366-arch-1.94.100.ft9366.20261021-1-x86_64.pkg.tar.zst`.

## Rollback artifact

The cached Arch package `libfprint-1.94.100-2-x86_64.pkg.tar.zst` was copied
with its detached signature into ignored `artifacts/rollback/` before any
installation. The signature verifies as a good signature from Arch packager
Antonio Rojas, key `1519D5ABA65BF6FC2B73C7567A4E76095D8A52E4`; Pacman records it
as valid/full trust. Its recorded SHA256 is
`567cc1f596d25ac7f315042a39f740a8030c5d2e73e5c813eb6b215ba41f73a3` and the
signature file SHA256 is
`6fd448e9861d680256ac4f01ec1175e48cacb76c4a7b92d5a7164b5b3567a7b0`.
The installed dependency check returned success when this package was verified.
Recheck the signature and current dependencies before use.

## Known limits

- The owner subsequently installed the experimental package through Pacman,
  replacing official libfprint. fprintd remains at 1.94.5-2.
- The built library discovery did not open the sensor.
- Owner-supplied terminal output confirms enrollment completion and one
  successful verification, including discovery of the stored right-index
  template by a subsequent client invocation. Repeated verification,
  wrong-finger rejection, restart persistence, and PAM integration are untested.
  PAM has not been modified.
- A capture from the separately approved reference experiment proves only that
  a 10240-byte frame was transferred to memory and cleared. It was not saved or
  inspected.
- One parallel test invocation had an intermittent unrelated Egis replay
  cancellation failure. A test-only fixture scheduling patch plus serial test
  execution then passed the full suite. No production Egis driver was changed.
- A previous temporary build directory showed differing ELF bytes across build
  paths. The current recipe does not claim bit-for-bit reproducibility; it pins
  sources and checksums and records the build environment and artifact hash.
- The udev rule persistently disables autosuspend for this one USB ID while the
  package is installed. No permission grant is added.

Next validation: repeated same-finger matches, wrong-finger rejection, and
suspend/resume behavior. See [hardware validation](hardware-validation.md).
