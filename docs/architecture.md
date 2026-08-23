# Architecture

```text
Application / PAM -> fprintd -> libfprint -> FT9366 adapter
                                            |
                               transport / state machine
                                            |
                                  USB bridge -> sensor
```

Only `driver/ft9366-frame.c` currently exists as an original offline component.
It validates a complete frame before decoding big-endian samples; it neither
transfers USB data nor performs image enhancement or matching.

Proposed layers: bounded transport, explicit initialization/capture states,
frame decoder, separately evaluated matcher, thin libfprint adapter. Reference code
uses FpDevice and custom host matching. FpImageDevice remains a research option.
No device-side matching or kernel module requirement is established.

Design open/close failure cleanup, disconnect, timeout, cancellation, recovery,
and suspend/resume before enabling hardware acquisition. Do not log biometric data.
