# USB transport core

Track B, original implementation. This is a backend-neutral asynchronous transfer
core, with a separate [libfprint adapter](libfprint-usb-adapter.md). Tests use a simulated backend only.
No real transfer has been submitted. Endpoints 0x01 OUT and 0x82 IN come from the
observed descriptors; no command payload or initialization sequence is included.

## API and ownership

Initialize ft9366_transport once per lifetime with backend submit/cancel functions.
The object is single-owner: serialize API calls and completion callbacks on one
event context. Keep the object and backend context alive until pending callbacks
have drained. No thread synchronization or heap allocation is provided.

start accepts 1..10240 bytes and a nonzero millisecond timeout. Writes are copied;
reads must supply a NULL input pointer. A second submission while active is rejected.
A failed backend submission enters IO_ERROR without expecting a callback. Return OK
from start means a valid request was accepted by the core; inspect state for an
immediate backend failure. No automatic retry is performed, especially after writes.

The backend borrows the buffer until exactly one completion callback. It must not
invoke completion inline from submit, and must never access the buffer after
completion. The backend enforces transfer deadlines; this core has no timer source.
The separate adapter forwards timeout values and maps cancellation and errors;
real USB behavior and a full libfprint build remain pending.

Successful completion requires the requested byte count for both reads and writes.
A short or oversized response fails closed with LENGTH_ERROR. Protocol semantics
and expected reply contents belong to the later command/state layer.

## Cancellation and stale callbacks

Cancellation, timeout requests or disconnect requests enter CANCELLING and ask
the backend to cancel. Buffers cannot be reused or erased until the completion
callback acknowledges the transfer has drained. If success races with cancellation,
the requested terminal reason wins and data is discarded. A backend that never
completes cancellation keeps the object blocked; it must not be freed to force reuse.

Each submission gets a monotonically increasing 64-bit token. Mismatched or duplicate
callbacks cannot transition a new operation. Token exhaustion rejects further work.
This guards core state; it cannot make a backend writing after completion safe.
Do not reinitialize a live object or bypass its lifetime rules.

## Results and erasure

A successful read retains bytes until read copies them to a sufficiently sized
caller buffer and release clears internal storage. Release is mandatory before
another request. Writes and failures clear storage immediately after completion.
Volatile byte stores clear the local buffer; caller copies, USB/library buffers,
swap and other memory are outside that guarantee. No payload is logged.
Input/output buffers must not overlap transport storage.

## Verification and remaining integration

Synthetic tests cover endpoint selection, owned write copies, bounded lengths,
timeout forwarding, full-size frames, short transfers, submit failures, error
propagation, cancellation races, stale callbacks, duplicate completions and token
exhaustion. GCC and Clang ASan/UBSan runs passed; leak detection was disabled due
to the previously documented sandbox restriction.

Before hardware use, build the adapter inside libfprint and test its lifecycle
with actual device operations after approval. Establish approved command sequences. The transport does not
identify response types or turn arbitrary bytes into image frames.
