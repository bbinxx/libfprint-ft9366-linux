# Compatibility

| Component | Evidence | Status |
| --- | --- | --- |
| USB 2808:a658 | local descriptors | detected |
| Physical open / claim / release / close | owner-supplied sudo probe output, November 14, 2026 | successful; standalone libusb probe |
| libfprint 1.94.100-2 / fprintd 1.94.5-2 | pacman inventory | installed |
| FT9366 Track A source with libfprint 1.94.100 | clean Arch package build; all 130 upstream tests pass | compiled; test suite does not validate sensor |
| Public libfprint ABI / fprintd | built SONAME and exported symbols compared; package metadata provides libfprint-2.so=2-64 | compatible at inspected ABI level; not installed |
| FT9366 driver discovery | built-library `FpContext` enumeration | discovered as `ft9366`; device not opened by enumerator |
| Reference initialization / touch flag / frame acquisition | owner-supplied approved capture output, November 14, 2026 | one successful reference-based capture; image quality unverified |
| Original frame decoder | synthetic GCC/Clang tests | passes offline checks |
| Track A reference initialization / enrollment / verification | initialization/capture owner output; no fprintd enrollment test | one reference frame transferred; matching/authentication unverified |
| Original Track B libfprint initialization / enrollment / verification | no hardware test | not implemented |
| Other USB IDs | no evidence | unsupported |

ABI target is libfprint-2.so.2. Package build, metadata, discovery and API ABI
checks pass for the observed version, but the replacement remains uninstalled.
Internal driver APIs can change across releases. Do not infer reliable hardware
authentication from compilation or device discovery.
