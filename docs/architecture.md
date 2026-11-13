# Architecture

```text
Application / PAM -> fprintd -> libfprint -> FT9366 adapter
                                            |
                               transport / state machine
                                            |
                                  USB bridge -> sensor
```

Original offline components are `driver/ft9366-frame.c` and
`driver/ft9366-capture.c`, plus a backend-neutral
[asynchronous transport core](transport.md) in `driver/ft9366-transport.c`.
It validates a complete frame before decoding big-endian samples; it neither
transfers USB data nor performs image enhancement or matching.

Proposed layers: bounded transport, explicit initialization/capture states,
frame decoder, separately evaluated matcher, thin libfprint adapter. Reference code
uses FpDevice and custom host matching. FpImageDevice remains a research option.
No device-side matching or kernel module requirement is established.

Design open/close failure cleanup, disconnect, timeout, cancellation, recovery,
and suspend/resume before enabling hardware acquisition. Do not log biometric data.

## Receive state contract

The capture core progresses IDLE -> RECEIVING -> READY -> IDLE. The future USB
adapter must identify the start and length of an actual frame response before
feeding its payload; arbitrary USB replies must never be treated as image bytes.
It can feed fragments and consume exactly one decoded frame. Oversized data enters
a protocol error state. Cancellation, timeout and disconnect are explicit terminal
states. All terminal paths clear the internal receive buffer using volatile stores.
This does not erase buffers owned by libfprint, USB helpers or the caller.

Only one owner may mutate a capture instance. Before resetting after an error,
the adapter must cancel/drain outstanding transfers so stale callbacks cannot
enter a new operation. The core does not schedule timeouts or implement USB
cancellation itself; the adapter must report those events and enforce deadlines.

The [libfprint USB adapter](libfprint-usb-adapter.md) implements the transport backend
with FpiUsbTransfer and per-transfer GCancellable objects. It is tested offline
against real headers; device registration and open/close remain separate work.
