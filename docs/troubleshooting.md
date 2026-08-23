# Troubleshooting

- USB absent: inspect lsusb and connection state; bus numbering can change.
- Permission failure: record exact error; do not add world-writable rules.
- fprintd NoSuchDevice: distinguish unsupported installed driver from USB absence.
  Daemon inactivity exit alone is not evidence of a daemon fault.
- Initialization/timeouts: record state and redacted transfer metadata; do not
  change command bytes or power settings randomly.
- Enrollment/verification: not implemented here; gather sanitized errors once tested.
- Autosuspend: current suspended state is normal evidence, not a demonstrated cause.
- Package conflicts: inspect metadata and transaction; never overwrite /usr/lib.
- Rollback: use a signature-verified official package through pacman -U only after
  reviewing dependencies and approval. No replacement has been installed yet.

Keep passwords and the administrative recovery session available. Do not edit PAM,
restart SDDM or reboot to troubleshoot an unverified driver.
