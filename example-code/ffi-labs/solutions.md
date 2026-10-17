# FFI exercise solutions

Source IDs identify files, not printed chapter numbers. The 32 answers below
correspond to the four exercises in each of sources 69–76.

## 69 — Foundations and semantic contracts

1. **Range.** `(19,23)` reaches native addition and writes 42. `(INT32_MAX,1)`
   reaches native addition but returns range failure and leaves 99. `(2**80,0)`
   fails Python-to-int32 conversion before the native call; the hypothetical
   native output remains 99. Valid arguments and a representable result are
   different checks.
2. **Extent.** A followed by U+00E9 is two code points but three UTF-8 bytes,
   hexadecimal `41 C3 A9`. A length of two interpreted as bytes truncates the
   final multibyte sequence; a byte length of three interpreted as a code-point
   count falsely describes the supplied text. Neither unit generally counts
   grapheme clusters.
3. **Effect.** The first load returns 4 and the second must return 9 after the
   callback. Common-subexpression elimination that reuses 4 changes the result.
   The visitor's effect summary must include allowed callback effects, even if
   the visitor contains no direct store to `g`.
4. **Retention.** Identify the object owner, how it stays reachable, how its
   address remains stable or is re-resolved, allowed access/mutation, executing
   threads, and the exact event acknowledging last use. Cancellation request is
   not necessarily last use. The wrapper must retain the required root/export/
   allocation until that event and release through the matching mechanism.

## 70 — ABI classification and lowering

1. **Eight.** Arguments 1–6 occupy RDI, RSI, RDX, RCX, R8, and R9. Arguments 7
   and 8 use the outgoing stack area according to ABI layout. The weighted sum
   is `1+4+9+16+25+36+49+64 = 204`. Stack placement is not modeled by merely
   reporting that the last two arguments are stack arguments.
2. **Rollback.** Pair needs one integer register and one SSE register. All six
   integer argument registers are occupied, so the entire Pair goes to the
   stack. It consumes no SSE slot; the final double uses XMM0, not XMM1.
3. **Hidden return.** The result-storage pointer occupies RDI. Five explicit
   integers use RSI, RDX, RCX, R8, R9; the sixth explicit integer uses the stack.
4. **Promotions.** The float becomes double and signed char becomes int on this
   target. The values remain 1.25 and 7. Store them in actual double and int
   objects and pass those objects' addresses. A descriptor for an eight-byte
   double cannot read a four-byte float object correctly; the char similarly
   lacks the size and representation of its promoted int.

## 71 — Layout and marshaling

1. **Reorder.** Pointer offset 0, tag offset 8, count offset 12, total 16,
   alignment 8. Padding moves to bytes 9–11; total size is unchanged. Reordering
   does not always reduce padding.
2. **Packet.** `09 03 00 00 00 41 00 42`. The header says three payload bytes.
   Removing the final byte leaves only two, and appending a byte leaves four.
   Both violate exact extent agreement. The internal zero is preserved as data.
3. **Overflow.** `(4,0)` passes `4 <= 4` and `0 <= 4-4`; no bytes are copied.
   `(1,SIZE_MAX)` passes the offset test but fails `SIZE_MAX <= 3`. Computing
   `1+SIZE_MAX` first would wrap to zero in unsigned size arithmetic and can
   incorrectly satisfy an addition-based upper-bound test.
4. **View.** Selected byte offsets are 0, 8, 12, 20; strides are 12 between rows
   and 8 between selected columns. Four contiguous int32 values instead use
   offsets 0, 4, 8, 12, including unwanted column one and omitting row one's
   column two. Copy/repack, use a stride-aware routine, or reject the view.

## 72 — Ownership and GC

1. **Generation.** Releasing `(0,0)` leaves slot zero empty with generation one.
   Reuse returns `(0,1)` for 99. Old lookup fails; new lookup returns 99. A
   finite-width generation must not wrap to a value still carried by an old
   handle; retiring exhausted slots is one possible policy.
2. **Root versus pin.** A reachable object can move. A pin prevents movement
   for its documented interval but, unless that runtime's pin API also retains
   a root, does not alone establish reachability. Both obligations must be
   satisfied. A handle permits pointer updates, but a cached derived pointer
   still needs stability during use.
3. **Close.** First close frees the payload and sets null. Second close sees
   null and performs no payload release. Read rejects the closed state.
   Finalization frees no payload, though Python also frees the small owner
   record. Exactly one nonnull payload release occurs.
4. **Cycle.** Native registration retains callback; callback captures wrapper;
   wrapper owns registration. Explicit close unregisters and drains callbacks,
   then releases the native retention of the callback, breaking that cycle.
   Waiting for wrapper unreachability without breaking the native root need not
   work. A weak-edge design is another option only with specified expiry rules.

## 73 — Callbacks and threads

1. **Nested.** Inner visits at outer indices 0, 1, 2 produce sums 0, 1, 3.
   The outer sum is 4. Independent stack contexts keep the nested counters and
   runtime callback references separate.
2. **Stop.** Start accepting with active zero. Enter gives active one. Stop
   disables admission. First collect is rejected because active is one and
   leaves the state unchanged. Leave produces active zero. Second collect
   succeeds. A later enter is rejected. The enumerator checks legal trace
   prefixes; this exercise explicitly continues after a rejected request.
3. **Self-wait.** The callback contributes one to active and cannot decrement
   it until it returns. Waiting inside it for zero prevents that return. Use a
   nonblocking stop and defer collection, or prohibit blocking self-retirement.
4. **Thread.** A native mutex does not establish an attached Python thread state
   or the correct interpreter. Free-threaded builds still require runtime
   attachment and valid object lifetimes; GIL builds also require the GIL for
   ordinary Python API use. Holding a mutex across arbitrary Python execution
   can additionally deadlock if a callback reenters the protected native service.

## 74 — Errors and unwinding

1. **Status.** On range failure the output is unchanged, so 99 is merely the old
   sentinel, not an arithmetic result. A caller that ignores status could
   mistake failure for a successful sum of 99. The output may even have been
   uninitialized in another caller; the status gates reading it.
2. **Identity.** A caller can compare the caught error object with its marker
   table and inspect its fields. A string replacement fails identity comparison
   and loses structure. The Lua binding preserves the actual object across the
   native status-return path.
3. **Cleanup.** Owner allocation failure: release input export. Payload failure:
   free owner and release input export. Capsule publication failure: the input
   export has already been released; free payload and owner. On publication
   success, capsule destruction owns payload/owner cleanup. Preserve the original
   failure while running cleanup.
4. **Cancellation.** Wait for terminal completion/cancellation acknowledgement
   establishing that the worker and callbacks no longer use the buffer. A
   cancellation request may be queued while a worker is actively reading; freeing
   immediately creates a use-after-free. Exactly one terminal transition releases
   retained resources if completion races with cancellation.

## 75 — Native extension models

1. **Version.** Version one and exact size returns the table. Version two
   returns null. Version one requiring one extra byte also returns null. The
   client validates returned metadata and required entries before invocation.
2. **Unload.** The destructor can reside in the plugin, and the object can rely
   on plugin globals, allocator state, or vtables. Retain the library through
   object destruction and all active calls/callbacks. Successful symbol lookup
   provides no guarantee after unloading.
3. **State.** Counters are 2 and 1. A process-global mutable counter would expose
   3 through both module instances. Per-module storage and correct receiver
   lookup isolate the counters. This test does not by itself prove every
   subinterpreter/concurrency configuration safe.
4. **Retry.** The charge may have happened despite the lost reply. Repeating it
   can charge twice. A durable operation ID with server-side deduplication and
   result recovery can support safe retries if recording the effect and its ID
   has suitable atomicity/durability. Merely numbering requests in volatile
   client memory is insufficient across worker restart.

## 76 — Implementation case study

1. **Plan copy.** `ffi_cif` points at the plan's embedded argument-type array.
   A structure assignment copies that pointer value; it does not rebase it to
   the destination array. The new structure can retain references into expired
   or modified old storage. Keep the plan address-stable or prepare a fresh plan.
2. **Errors.** `2**80` fails Python integer conversion with `OverflowError`
   before native invocation. Representable inputs whose sum overflows fail in
   `bb_add`; the outer wrapper maps the range status to `OverflowError`. A
   callback exception is detected by the Python callback adapter, reported as
   native callback failure, and propagated as the original exception after the
   visitor returns. No partial sum is published.
3. **Copies.** Constructor copies N bytes from input to native storage; readback
   copies N bytes to Python bytes or a Lua string. Total payload traffic is 2N
   bytes of copying, hence O(N) time and O(N) additional storage for each retained
   copy (plus constant owner metadata). Original, native, and returned copies
   can all coexist. Closing frees native payload independently of readback.
4. **Cache.** Include ABI identifier, result kind, ordered argument kinds, and
   argument count; a generalized interface also needs aggregate layouts and
   variadic fixed/total counts. Publish immutable prepared metadata at a stable
   address and retain it through every active call before eviction. Target code
   has a separate retention obligation. Same-count signatures can use different
   banks and sizes, so count alone is not a safe key.
