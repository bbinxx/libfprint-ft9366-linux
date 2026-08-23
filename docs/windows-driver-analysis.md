# Windows driver analysis

Status: no Windows driver package obtained or analyzed. No binary findings exist.

Search official ASUS K3402ZA support, hardware-vendor resources, then Microsoft
Update Catalog for USB VID_2808&PID_A658. Record source URL, retrieval date,
package version, publisher/signature, archive SHA256 and extraction method.
Inspect archives before extraction and never execute drivers on the Linux host.

INF review: hardware IDs, service names, interfaces, registry settings, firmware,
dependencies and power settings. Static binary review: imports, strings, transfer
paths and state transitions. Record tool versions and reproducible offsets/hashes.
Separate hypotheses from confirmed trace behavior. Do not distribute vendor binaries
or upload them without permission; record redistribution terms first.
