# Linux distribution compatibility

Project name: `libfprint-ft9366-linux`. Only Arch Linux has actual build and
hardware evidence. Other distributions are unverified and require native
packaging plus checks against their libfprint/fprintd versions.

| Component | Evidence | Status |
| --- | --- | --- |
| USB 2808:a658 | local descriptors | detected |
| Physical open / claim / release / close | owner-supplied sudo probe output, November 14, 2026 | successful; standalone libusb probe |
| libfprint-ft9366-arch 1.94.100.ft9366.20261021-1 / fprintd 1.94.5-2 | Pacman inventory after owner-approved installation | installed; official libfprint preserved as rollback |
| FT9366 Track A source with libfprint 1.94.100 | clean Arch package build; all 130 upstream tests pass | compiled; test suite does not validate sensor |
| Public libfprint ABI / fprintd | built SONAME and exported symbols compared; package metadata provides libfprint-2.so=2-64 | installed; compatible at inspected ABI level |
| FT9366 driver discovery | built-library `FpContext` enumeration | discovered as `ft9366`; device not opened by enumerator |
| Reference initialization / touch flag / frame acquisition | owner-supplied approved capture output, November 14, 2026 | one successful reference-based capture; image quality unverified |
| Original frame decoder | synthetic GCC/Clang tests | passes offline checks |
| Track A enrollment / verification | owner terminal output, November 14, 2026 | enroll-completed and one verify-match; repeated reliability unverified |
| Original Track B libfprint initialization / enrollment / verification | no hardware test | not implemented |
| Other USB IDs | no evidence | unsupported |

ABI target is libfprint-2.so.2. Package build, metadata, discovery and API ABI
checks pass for the observed version. The owner installed the replacement and
completed one enrollment and one verification.
Internal driver APIs can change across releases. Do not infer reliable hardware
authentication from compilation or device discovery.
