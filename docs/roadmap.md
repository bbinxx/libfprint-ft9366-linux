# Roadmap

1. Evidence inventory: initial report available; Nix version resolution and full
   attribution audit remain open.
2. Original offline modules: frame decoder and capture state machine tested; USB adapter next.
3. Track A: review integration patch, dependency approval, non-installed build.
4. Protocol research: official Windows sources, reviewed captures and register map.
5. Original USB integration: reviewed operations, init/capture/cancel/recovery tests.
6. Packaging: reproducible recipe, ABI/lint/chroot checks, verified rollback.
7. Approved hardware testing: discovery, enrollment, repeatable verification.
8. Authentication: one reviewed PAM target at a time with password fallback.
9. Publication: license selection/compliance and explicit GitHub/AUR approval.

Track A and Track B proceed separately; no milestone substitutes for hardware tests.

## Feature branch sequence

Create each branch from current dev when its prerequisites are ready. Keep its
scope separate; never mark a hardware milestone done from an offline test.

| Branch | Deliverable / completion gate |
| --- | --- |
| feature/usb-initialization-research | Source sequence audit; exact response semantics and local traces remain open |
| feature/usb-transport | Offline transfer core tested; hardware validation pending |
| feature/libfprint-usb-adapter | Concrete FpiUsbTransfer adapter compiled against real headers and simulated; full library build pending |
| feature/device-lifecycle | Interface claim/close/drain and failure cleanup tested offline; sensor initialization remains pending |
| feature/image-acquisition | Connect capture state and decoder to verified frame responses |
| feature/libfprint-integration | Build and register original adapter inside pinned libfprint |
| feature/enrollment-matching | Template format, enrollment flow and evaluated host matcher |
| feature/arch-packaging | Reproducible build, ABI/lint checks and verified rollback |
| feature/hardware-connection | Owner reports successful privileged open/claim/release/close; sensor commands not yet tested |
| feature/hardware-validation | Approved discovery, enrollment, verification and failure tests |
| chore/project-license | Owner-approved license and complete provenance audit before publication |
| feature/pam-integration | Only after reliable hardware verification and explicit approval |

Merge each reviewed increment into dev with --no-ff. Research can land with clearly
recorded unresolved questions; implementing hardware writes must wait for their
evidence and approval gates. Do not create empty branches for future tasks.
