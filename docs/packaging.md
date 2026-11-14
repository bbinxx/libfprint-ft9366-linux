# Arch package: Track A baseline

This is an independently maintained Arch integration around pinned upstream
`libfprint` and FT9366 driver sources. It does not import upstream Git history,
and the bundled driver is not original Track B code. See
[`UPSTREAM-NOTICE`](../packaging/arch/UPSTREAM-NOTICE) for provenance.

## Build

From the repository root, run:

```sh
python3 scripts/build_arch_package.py
```

The script copies only `packaging/arch/` into a fresh `/tmp` directory and runs
`makepkg -s --log` without `-i`. Makepkg fetches immutable source archives and
checks their BLAKE2 checksums before applying the local integration and test
fixture patches. The build uses `SOURCE_DATE_EPOCH=1792590607` by default. It
does not modify the system package database. It stores the package and a JSON
build summary in ignored `artifacts/arch/`; build logs and the full source tree
remain under `/tmp`.

Required build/check dependencies are listed in `packaging/arch/PKGBUILD`.
On the initial build they were installed by the owner: `meson`,
`gobject-introspection`, `gtk-doc`, and `umockdev`. The recipe also declares the
official Arch libfprint build dependencies. Do not add downloads to `build()`.

## Current verified result

Build evidence collected 2026-11-14 on Arch x86_64, GCC 16.2.1, Meson 1.12.1:

- Pinned `libfprint` commit: `80a4b5ec612892c5c056c48dddaa561452cf37ec`.
- Pinned FT9366 reference commit: `5420cd9b7124e5a70deaae4bffedd969ae4ef91c`.
- Meson build completed and all 130 upstream tests passed in serial mode.
- `namcap` reported no issues for recipe or package.
- The package exports the installed-compatible SONAME and exact public ABI
  symbol set; `fprintd`'s `libfprint-2.so=2-64` dependency is provided.
- `fprint-list-supported-devices` lists `2808:a658` as
  `FocalTech FT9366 (RTS5811 bridge)`.
- A small public-API enumerator loaded the built library and discovered that
  driver without installing the package. It did not open the sensor.
- Package contents include both upstream license texts, provenance notice, and
  a device-ID-specific udev rule. `pacman -Qip` reports no install script.
- The package was built natively, not in a clean chroot. The local package is
  unsigned. Artifact SHA256 is recorded by the build script's stdout; rebuilds
  can differ in ELF bytes because build paths are embedded by upstream test
  helper configuration. No bit-for-bit reproducibility claim is made.

The upstream `egis_etu905` cancellation replay was intermittent under parallel
test scheduling. The recipe applies a narrow patch to that test fixture and
runs the suite serially. It changes no production Egis code. The final serial
run passed 130/130. The FT9366 hardware is not involved in that test.

The udev rule sets `power/control` to `on` only for USB VID:PID `2808:a658`.
This persistent package behavior is intended to avoid autosuspend during
operation and must be considered when reviewing installation. It grants no
new device permissions. The FT9366 driver code itself also includes the
reference integration's initialization sequence; do not infer enrollment
reliability from compilation, enumeration, or frame transfer alone.

## Package metadata and rollback

The generated package provides `libfprint=1.94.100` and
`libfprint-2.so=2-64`, and conflicts with the official `libfprint`. `fprintd`
remains installed. The package's exact metadata is inspectable with:

```sh
pacman -Qip artifacts/arch/libfprint-ft9366-arch-1.94.100.ft9366.20261021-1-x86_64.pkg.tar.zst
namcap packaging/arch/PKGBUILD artifacts/arch/*.pkg.tar.zst
```

The official `libfprint 1.94.100-2` rollback archive and detached signature
are preserved in ignored `artifacts/rollback/`. The signature was verified
against Arch's pacman keyring; the copied archive hash is recorded in
`docs/arch-build-report.md`. Before any installation, recheck its signature,
dependencies, proposed Pacman transaction and the exact package files; see
[`installation.md`](installation.md). The rollback package is not a substitute
for explicit approval.

## Recipe maintenance

`packaging/arch/PKGBUILD` is authoritative; regenerate `.SRCINFO` after any
metadata change:

```sh
cd packaging/arch
makepkg --printsrcinfo > .SRCINFO
```

Update the immutable source revision, checksum, upstream notice, compatibility
report, and tests together. Rebuild the complete package after patch or source
changes. Do not install with `makepkg -i`. Do not publish an AUR entry or remote
release until license review and real-hardware enrollment/verification evidence
are complete.
