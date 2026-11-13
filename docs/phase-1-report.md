# Phase 1: environment and compatibility report

Inspection: 2026-11-13 to 2026-11-14, Asia/Kolkata. No package installation, library replacement,
PAM edit, daemon restart, USB reset, or experimental USB transfer was performed.
No driver was written, built, or tested. Enrollment and verification remain untested.

## Workspace and independence

The project already exists with an unborn `dev` branch, no commits, and no remote.
Existing untracked files were preserved: empty README.md, .gitignore, and three
empty documentation files. No upstream history was imported. No AGENTS.md was
found under the Driver directory. The existing reference checkout is clean.

The workspace is on fuseblk (mount options include nosuid,nodev). A temporary,
self-cleaning probe confirmed symlink creation/read and git init. chmod(0600)
left the test file at 0777; Unix mode protection is not effective here. Existing
Git has core.filemode=false. Use a native Linux filesystem for builds and any
private capture material; do not store biometric material in this workspace.
Normal add/commit operations have not yet been exercised in the project.

## Verified environment

- /etc/os-release: Arch Linux; kernel 7.2.15-arch1-1.
- DMI product: VivoBook_ASUSLaptop K3402ZA_S3402ZA.
- libfprint 1.94.100-2; fprintd 1.94.5-2, both recorded by pacman as signature validated.
- Installed library SONAME: libfprint-2.so.2. fprintd requires libfprint-2.so=2-64.
- A matching official package and .sig exist in /var/cache/pacman/pkg;
  these are rollback candidates, not yet independently signature/dependency verified.

| Tool/package | Observed version or status |
| --- | --- |
| git | 2.56.1-1 |
| gcc | 16.2.1+r23+gd564253eb6c8-1 |
| clang | 23.1.4-1 |
| make | 4.4.1-3 |
| ninja | 1.13.2-3 |
| pkg-config/pkgconf | pkgconf 3.0.10-1 |
| makepkg | executable present |
| base-devel | 1-2 |
| meson | missing |
| devtools / pkgctl | missing |
| gobject-introspection | missing |
| gtk-doc | missing |
| umockdev | missing |

Other official build/check dependencies present: glib2-devel 2.90.1-1,
python-cairo 1.29.2-1, python-gobject 3.58.1-1, systemd 262.2-1, cairo 1.18.6-1.
Runtime dependency packages present: libgcc, glib2, glibc, libgudev, libgusb,
openssl, pixman. libgusb is 0.4.9-2; openssl is 4.1.0-1.
No dependency installation was attempted. A full build will require approval to
install missing packages; devtools is also useful for arch-meson and clean chroots.

## Hardware observations

lsusb initially failed inside the sandbox (libusb -99). Approved inspection outside
it identified Bus 003 Device 007, VID:PID 2808:a658. lsusb -v reported
"Couldn't open device, some information will be missing"; cached descriptors were
available. No elevated root access or interface claim was attempted.

| Descriptor | Observed value |
| --- | --- |
| Manufacturer | Realtek USB2.0 Finger Print Bridge |
| Product | FocalTech Fingerprint Device |
| bcdDevice | 1.12 |
| USB / speed | USB 2.00 / high speed 480 Mbps |
| Configuration | one; bus powered; remote wakeup; maximum 100 mA |
| Interface | 0, alternate 0; class/subclass/protocol ff/ff/ff |
| Endpoints | bulk OUT 0x01; bulk IN 0x82 |
| Maximum packet | 512 bytes each; EP0 64 bytes |
| Serial string | none advertised |

Sysfs path at inspection: /sys/bus/usb/devices/3-8. power/control=auto,
runtime_status=suspended, autosuspend_delay_ms=2000. This does not establish a
power-management fault. Do not change it without a reviewed experiment.
Descriptors establish the USB identity, not the silicon model: FT9366/RTS5811
identification is consistent with the reference project, but not independently
read from a chip identity register.

## Official Arch source

[Official packaging repository](https://gitlab.archlinux.org/archlinux/packaging/packages/libfprint)
was cloned separately into /tmp/ft9366-arch-packaging at tag 1.94.100-2.
Packaging commit: 4754ff229c0e4cb566f33e99e962d702bfc52970.
Annotated packaging tag object: 5427fdbefce1b2fcabc779944e008caaf3c2ecb2.

PKGBUILD selects signed upstream tag v1.94.100:
[libfprint source](https://gitlab.freedesktop.org/libfprint/libfprint/-/tree/v1.94.100).
The separately cloned source at /tmp/ft9366-libfprint-source resolves to commit
80a4b5ec612892c5c056c48dddaa561452cf37ec; annotated tag object
b743c1e2237559390b4e139eda8dc07f95c0e6b7.
This is the matching recipe source, not an independent reproduction of the installed binary.
The recipe requires signing key D4C501DA48EB797A081750939449C2F50996635F.
git verify-tag returned 0 with GOODSIG and VALIDSIG after importing the Arch
recipe key into a temporary /tmp keyring. Signing subkey:
43B03E28326C54CFA2EC66A340F65066AD7C16DB, primary fingerprint matching
the recipe. Initial verification failed because the normal keyring was read-only
and lacked the key; no user keyring was modified. Key ownership trust remains
undefined; verification establishes consistency with the key supplied by Arch.

The recipe applies no source patches (prepare only changes directory). It uses
arch-meson, -Ddrivers=all, -Dinstalled-tests=false, meson compile, meson test
--print-errorlogs, and staged meson install. Retain its dependencies and checks.
Do not finalize provides/conflicts until a replacement package is built and inspected.
The installed SONAME suggests provides libfprint plus generated libfprint-2.so
and conflicts libfprint will be appropriate, but this is not yet package validation.

An additional official archive was fetched while Git transfer was delayed:
/tmp/ft9366-libfprint-v1.94.100.tar.gz, SHA256
edc90e02f330a7595ceaf37f2c6ec32ed43541347fe936d3273b0bb2524fd19c.
That local hash records the artifact; it is not independent authenticity verification.

## Reference implementation and licensing

[Reference repository](https://github.com/ilyamiro/ft9366-fingerprint), existing checkout:
/home/bbin/fingerprint-driver-review/ft9366-fingerprint.
Local commit and live remote HEAD both:
5420cd9b7124e5a70deaae4bffedd969ae4ef91c.
Git shortlog attributes its two commits to ilyamiro; the latest commit includes a
Claude Sonnet 5.5 co-author trailer. Preserve actual upstream attribution if reused.
LICENSE contains LGPL 2.1; README and ft9366.c explicitly allow LGPL-2.1-or-later.
A complete per-file provenance audit remains necessary before redistribution.
No reference code was copied into this project.

The Nix overlay takes libfprint as an input rather than specifying its own version.
flake.lock pins nixpkgs 7fd4004d0511991fff3a0524856a9be09e28c5f0;
the exact libfprint derivation at that nixpkgs revision has not yet been resolved.
It copies five files into libfprint/drivers. ft9366.c directly includes the matcher C
file, so the matcher is not separately registered as a translation unit.
It inserts a source entry in libfprint/meson.build and a driver entry in root
meson.build. Both nb1010 insertion anchors exist in v1.94.100.
It removes a658 from the known-unsupported table and adds it to the hwdb generator's
autosuspend denylist, also removing a generated hwdb entry. These lists control
hwdb generation, not USB driver discovery. An Arch patch must target these exact
roles and preserve other devices. The NixOS module additionally sets power/control=on
with a narrowly matched USB udev rule and overrides fprintd's libfprint dependency.
Its autosuspend recommendation is an upstream claim, not a locally demonstrated fix.

## API compatibility and proposed architecture

Static checks found all 39 referenced libfprint function names in the release's
top-level headers (excluding local type functions). Relevant APIs include FpiSsm,
FpiUsbTransfer, fpi_device_action_is_cancelled, feature initialization,
FpDeviceClass.temp_hot_seconds, and enrollment/verification completion callbacks.
This verifies name availability only, not every signature or compilation.
No compatibility error has been demonstrated because no build was attempted.

Reference architecture: FpDevice USB subclass, asynchronous state machines,
interface 0, reset on open, script-based initialization, frame acquisition,
custom host-side matching, FPI_PRINT_RAW and versioned GVariant template data.
It implements open/close/enroll/verify/identify. It has no explicit cancel,
suspend, or resume class hook; polling checks cancellation, but USB script
submissions use NULL cancellables. Review cancellation latency and resource
cleanup, including reset/claim/open failure paths, before hardware testing.

Recommended original architecture: a bounded command/response transport layer,
explicit initialization/capture states, separately testable frame decoder and
matcher, and a thin FpDevice adapter if custom matching remains necessary.
FpImageDevice remains an option only after testing whether standard image matching
can use these small frames. No evidence currently supports device-side matching
or a kernel module requirement. libfprint's internal driver APIs require building
inside its source tree; a standalone public-API plugin is not established here.

## Protocol evidence and unknowns

The following is reference-source evidence, not a hardware trace from this laptop.
Do not confuse script opcodes with on-wire commands.

| Item | Direction / endpoint | Format / meaning | Confidence and evidence |
| --- | --- | --- | --- |
| Transport | OUT 0x01 / IN 0x82 | Bulk transfers | Descriptor confirmed; reference agrees |
| Script record | host memory only | op byte, big-endian 16-bit length; op 0 write, 1 read, 2 delay | Confirmed in gen_script.py and interpreter |
| Frame | IN 0x82 | 64 x 80 x 16 bits = 10240 bytes | Reference ft9366.c; not locally captured |
| Finger status | IN 0x82 | reference tests flag bit 0x0002 | Reference FT_FLAG_DOWN; register semantics unverified |
| Initialization | OUT/IN | generated replay sequence, USB reset first | Reference source; individual register meanings unresolved |

Reference generator selects fixed event slices from an external tshark dump;
raw capture provenance and reproducibility have not been verified. Reads encode
lengths, not expected reply content. The script interpreter does not fully bound
write payloads against remaining script length; an original parser should reject
truncated headers/payloads, excessive lengths, unexpected responses, and unknown ops.
Frame processing subtracts an empty-sensor baseline and uses custom keypoints,
binary descriptors and ridge overlays. Matcher calibration on one person's fingers
is insufficient evidence of authentication reliability. Do not equate reference
success claims with local enrollment, verification, or security validation.

Still unknown: command/register semantics, mandatory vs incidental initialization
writes, chip identification, firmware requirements, challenge-response behavior,
reset necessity, response invariants, recovery, suspend/resume, and robust matcher
performance across users. No proprietary firmware dependency was demonstrated.
No Windows package has been obtained or analyzed during this Phase 1 pass; official
ASUS/Microsoft discovery and INF/static binary analysis remain separate research work.

## Recommendation and next gates

Use a parallel approach, with explicit separation:

1. Track A: independently authored Arch packaging around pinned reference sources,
   used only as a compatibility baseline. Do not call that an original driver.
2. Track B: evidence-led original protocol implementation, with source references
   recorded and new offline parsing/state-machine tests before USB experiments.

First resolve missing build tools and retain source signature checks; prepare reviewed
Arch integration patches and a non-installed build on a native Linux filesystem.
Resolve the Nix libfprint version and audit licensing before distributing baseline
sources. No driver implementation or package installation is approved by this report.
Before hardware experiments, review exact commands and obtain explicit approval.
Before replacing libfprint, verify the cached rollback package signature and current
dependencies, inspect the replacement transaction, and obtain installation approval.
Keep PAM untouched until repeatable enrollment and verification are demonstrated.

Phase 1 establishes a plausible source-compatible integration path, not a working
driver. Build, package ABI validation, device discovery, enrollment and verification
are all outstanding.
