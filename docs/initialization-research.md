# Initialization research: source audit

Observation date: November 14, 2026. Track B protocol research only.
No USB operation was executed. This report is not approval to replay a sequence.

## Reproduce

Reference checkout revision: 5420cd9b7124e5a70deaae4bffedd969ae4ef91c.
The header is read as text, never compiled or executed by this tool:

```sh
python3 scripts/ft9366_script_audit.py /home/bbin/fingerprint-driver-review/ft9366-fingerprint/driver/ft9366_script.h
```

The original audit tool validates the reference's host-side record boundaries and
emits only counts, lengths and hashes. It does not copy scripts into this repository,
interpret register meanings, validate device replies or access USB.

## Results

| Script | Records | Writes | Reads | Frame-sized reads | Scripted delay ms |
| --- | --- | --- | --- | --- | --- |
| init | 304 | 223 | 73 | 4 | 317 |
| idle | 65 | 46 | 18 | 0 | 50 |
| scan | 60 | 46 | 13 | 1 | 32 |

Initialization reads total 41117 bytes, writes total 1348 bytes. Read-length
histogram: 19×1, 41×2, 4×4, 5×8 and 4×10240 bytes.
The 317 ms are explicit container delays only, excluding transfer time and the
reference's separate 500 ms post-reset wait; they are not a measured init duration.

Decoded container SHA256 values:

- init: cd1ec047cd216c1b0ed26beff0186fb0cf5b429964fa9a647d4b422b4cc67ca0
- idle: 4ee101d95a0715813a3a3c30f662b53dc62409bd942916314255726f7e13178c
- scan: 735f193912cecb698d7545adba3d74e47be2d6a537cb32420ae3bff55e1a536c

## Lifecycle evidence

Reference ft9366.c dev_open resets the USB device, claims interface 0, waits
500 ms and executes init. INIT_CHECK requires at least four frames. The third
and fourth are averaged as the empty-sensor baseline; this is source behavior,
not proof those frames are always empty on this hardware. The source warns if
the baseline mean is low rather than establishing a calibrated invariant.
The script reader enforces read length but does not encode expected reply bytes.
Thus a successful replay length check would not establish correct initialization.

## Required evidence before original initialization

- Obtain a trace with known capture conditions and provenance; keep images private.
- Identify command framing, addressed registers and required response invariants.
- Separate bridge setup from sensor setup, calibration and optional repeated writes.
- Establish whether reset is required and how disconnect/claim failures unwind.
- Review the exact proposed operations with the owner before any USB experiment.

Until then an original adapter must fail closed rather than silently send the
reference replay. This research increment establishes container structure only.
