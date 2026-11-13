# Compatibility

| Component | Evidence | Status |
| --- | --- | --- |
| USB 2808:a658 | local descriptors | detected |
| Physical open / claim / release / close | owner-supplied sudo probe output, November 14, 2026 | successful; standalone libusb probe |
| libfprint 1.94.100-2 / fprintd 1.94.5-2 | pacman inventory | installed |
| Reference API names and Meson anchors | static source inspection | present, compilation untested |
| Original frame decoder | synthetic GCC/Clang tests | passes offline checks |
| Driver initialization / enrollment / verification | no test | unimplemented here |
| Other USB IDs | no evidence | unsupported |

ABI target is libfprint-2.so.2, but a replacement package must still validate its
exported ABI and dependencies. Internal driver APIs can change across releases.
Do not infer hardware support from the decoder or reference project's claims.
