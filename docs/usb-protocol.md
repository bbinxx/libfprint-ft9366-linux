# USB protocol evidence

The owner reports a real IN-only read timed out with 0 bytes. No sensor OUT
command or captured frame has yet been observed locally.
Reference revision: 5420cd9b7124e5a70deaae4bffedd969ae4ef91c in
[ilyamiro/ft9366-fingerprint](https://github.com/ilyamiro/ft9366-fingerprint).

| Item | Direction / endpoint | Payload / meaning | Evidence / confidence |
| --- | --- | --- | --- |
| Bulk transport | OUT 0x01, IN 0x82 | Vendor-specific interface 0 | USB descriptors, observed |
| Image frame | IN 0x82 | 10240 bytes, 64×80 big-endian 16-bit samples | ft9366.c frame_to_u16, source evidence only |
| Finger flag | IN 0x82 | bit 0x0002 tested by reference | FT_FLAG_DOWN, semantics unverified locally |
| Initialization | both | replayed sequence after reset | reference script; register meanings unknown |

Reference script records use a one-byte opcode and two-byte big-endian length.
These are host-side replay metadata, not on-wire command identifiers.
No hardware command semantics are sufficiently established for an original sender.

Current decoder validates exact size, pointers and output capacity before writing.
Its caller must provide valid non-overlapping buffers. Geometry and byte order
remain assumptions pending local capture. Synthetic tests confirm software behavior,
not actual sensor encoding, orientation, calibration or image quality.

For each future command record exact bytes, endpoint, transfer length, response
invariants, source/trace identifier, observation date and confidence. Review writes
before execution; keep private captures outside Git. Unknown: chip IDs, firmware,
authentication, initialization essentials, recovery and power transitions.

See [initialization source audit](initialization-research.md) for measured static
record counts and unresolved initialization gates.
