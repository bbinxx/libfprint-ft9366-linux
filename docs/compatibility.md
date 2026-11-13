# Compatibility

| Component | Evidence | Status |
| --- | --- | --- |
| USB 2808:a658 | local descriptors | detected |
| Physical open / claim / release / close | owner-supplied sudo probe output, November 14, 2026 | successful; standalone libusb probe |
| libfprint 1.94.100-2 / fprintd 1.94.5-2 | pacman inventory | installed |
| Reference API names and Meson anchors | static source inspection | present, compilation untested |
| Reference initialization / touch flag / frame acquisition | owner-supplied approved capture output, November 14, 2026 | one successful reference-based capture; image quality unverified |
| Original frame decoder | synthetic GCC/Clang tests | passes offline checks |
| Original libfprint initialization / enrollment / verification | no hardware test | unimplemented here |
| Other USB IDs | no evidence | unsupported |

ABI target is libfprint-2.so.2, but a replacement package must still validate its
exported ABI and dependencies. Internal driver APIs can change across releases.
Do not infer hardware support from the decoder or reference project's claims.
