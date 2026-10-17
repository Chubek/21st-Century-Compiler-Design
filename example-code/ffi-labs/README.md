# FFI and native-extension laboratories

Companions to stable chapter sources 69–76. Printed chapter numbers follow
`index.ltx`. All examples are original. Exercise answers are in `solutions.md`.

## Run

From the repository root:

```sh
PYTHONDONTWRITEBYTECODE=1 python3 -m unittest discover -s example-code/ffi-labs -p 'test*.py' -v
```

Requirements: Linux x86-64/System V ABI, GCC, Clang, Python 3.11+ with development
headers, `pkg-config`, libffi with closure support, Lua 5.4 with development
headers and a `lua5.4` executable, pthreads, and the native dynamic loader.
The Lua build must use 64-bit integers; CPython must be a conventional
GIL-enabled build. The native fixture assumes 32-bit C `int` and 64-bit pointers.
The tested host uses Python 3.14.7, Lua 5.4.9, and libffi 3.8.0.
Tests build in temporary directories under `/tmp/opencode` and remove them.

Run only the native/language matrix with `python3 example-code/ffi-labs/test.py`.
Run only the finite models with
`python3 -m unittest discover -s example-code/ffi-labs -p 'test_models.py' -v`.
These tests require no Python packages beyond the standard library.

To keep a Python module for interactive use (run from this directory):

```sh
mkdir -p /tmp/opencode/book-ffi-python
cc -std=c11 -O2 -Wall -Wextra -Wpedantic -Werror -fPIC -shared \
  $(pkg-config --cflags libffi) $(python3-config --includes) \
  bookffi.c python_module.c $(pkg-config --libs libffi) \
  -o /tmp/opencode/book-ffi-python/bookffi.abi3.so
PYTHONPATH=/tmp/opencode/book-ffi-python python3 check_python.py
```

To keep a Lua module, use a separate module directory:

```sh
mkdir -p /tmp/opencode/book-ffi-lua
cc -std=c11 -O2 -Wall -Wextra -Wpedantic -Werror -fPIC -shared \
  $(pkg-config --cflags libffi lua5.4) bookffi.c lua_module.c \
  $(pkg-config --libs libffi lua5.4) -o /tmp/opencode/book-ffi-lua/bookffi.so
LUA_CPATH='/tmp/opencode/book-ffi-lua/?.so' lua5.4 check_lua.lua
```

## Native contract

`bookffi.h` is the shared declaration. Native callers supply valid live objects,
correctly sized/aligned writable outputs, and readable input extents. Null and
range checks do not validate arbitrary nonnull addresses. Successful copy ranges
must not overlap. Calls to `bb_data`/`bb_size` require a live nonnull buffer.

- `bb_add(a,b,out)`: checked signed 32-bit addition with a widened intermediate;
  output unchanged on failure. Null output is invalid.
- `bb_new(data,size,out)`: copies up to 1,048,576 uninterpreted bytes into a native
  allocation. Null input is allowed only for size zero. Output is published only
  on success. The caller retains ownership of input data.
- `bb_read(buffer,offset,count,out)`: validates `offset <= size` and
  `count <= size-offset`, then copies; null output allowed only for zero count.
- `bb_data(buffer)`: borrowed read-only pointer, valid until `bb_free(buffer)`.
- `bb_free(buffer)`: release with the allocating component. Null is harmless;
  double-free of a stale nonnull pointer is invalid.
- `bb_visit(count,callback,context,out)`: synchronous, calling-thread callbacks
  for indices zero through count minus one. Count is in 0–10,000; callback
  writes int32 and returns status. Stop at first failure and preserve output;
  successful sum is int64. Callback side effects are not rolled back. No
  callback/context is retained after return. Each successful callback must write
  its output, and callers must not nonlocally exit across the core's frames.

Statuses are `BB_OK`, `BB_INVALID`, `BB_RANGE`, `BB_NOMEM`, and `BB_CALLBACK`.
Runtime wrappers translate them to their language's error channel.

`FxPlan` supports `FX_I32`, `FX_U64`, `FX_F64`, and `FX_POINTER`, at most eight
arguments, and `FFI_DEFAULT_ABI`. A prepared plan is **not copied or moved**:
its `ffi_cif` refers into its own type array. Keep it alive through each call.
Preparation failure marks it unready. Invocation validates count/kind metadata
but trusts the supplied target signature, live target code, and selected active
union members. Native output/argument objects must obey the callee's contract.
There is no arbitrary-signature parser, signature inference, or user-facing
raw-address call facility. Independent plans can be used concurrently; the
fixture does not establish concurrent mutation or re-preparation of one plan.

## Runtime interfaces

Both bindings expose `add`, `create`, and `visit`. Python exposes `bytes(owner)`
and `close(owner)`; Lua exposes `owner:bytes()` and `owner:close()`.

- Python `add` and visitor results require exact `int` objects in int32 range,
  rejecting booleans and subclasses. Lua requires the integer subtype, rejecting
  float `1.0` and numeric strings. These are deliberate binding policies.
- Byte construction copies, preserving zero bytes and byte 255. Python requests
  a contiguous buffer export and releases it after copying; strided input is
  rejected. Lua accepts strings without coercion. Both return independent copies.
- An owner is closed by freeing its payload and clearing the pointer. Repeated
  close is harmless; reading a closed payload fails. Python capsule destruction
  later frees the owner record. Lua uses full userdata with `__gc` and `__close`.
- Callbacks run synchronously on the attached calling thread. Original Python
  exceptions and Lua error objects are preserved. Nested calls have separate
  contexts. No library mutex is held across callback entry.
- Python uses multi-phase initialization and per-module counter state. Its
  Limited API floor is 3.11; the `abi3` filename expresses the intended ABI class,
  not a tested compatibility matrix. Reimported module-state independence is
  tested; subinterpreters, own-GIL interpreters, and free-threaded builds are not.
- Lua dynamic loading and a native embedded host use the same module entry.
  The host protects initialization/evaluation and closes its state on return.
  The module is built specifically against Lua 5.4, not the host's other Lua ABI.

## What the checks establish

GCC and Clang each build normal and AddressSanitizer/UndefinedBehaviorSanitizer
native executables. All use strict compiler warnings as errors. Tests cover:

- 49 signed-add boundary pairs with independent mathematical expectations;
  overflow sentinels and null output;
- mixed integer/SSE arguments, eight integer arguments, pointer identity;
- native size/alignment/offset agreement for Pair and direct-call comparisons
  for mixed-register Pair and memory-class Triple argument/return paths;
- promoted variadic doubles and rejection of an unpromoted float descriptor;
- 101 libffi closure calls with retained context and no users at release;
- native buffer boundaries, embedded zeros, invalid extents, callback early stop,
  failure-atomic sums, and reentrant bridge calls;
- four pthread workers making 1,000 calls each using independent plans;
- `dlopen`/`dlsym` negotiation, rejected version/size, and retained library lifetime.

The embedded Lua host is also sanitized. Dynamic Lua and Python extension
loading is tested under ordinary system interpreters for both compilers; those
interpreter processes are **not sanitizer-instrumented**. Runtime scripts check
values, copying, export release, repeated close, GC, scope close (Lua), nested
callbacks, original error identity, and independent module state (Python).

The Python models separately check ordinary field alignment, a restricted SysV
register-assignment profile, 195 byte-record round trips and all their truncated
prefixes, handle movement/stale-generation rejection, and all 16,384 length-seven
registration event words (checking legal prefixes). The registration transitions
are abstract atomic steps: they are not a tested concurrent native registry.

Not implemented: a production moving collector, zero-copy exports, foreign-thread
managed callbacks, asynchronous cancellation, a worker-process protocol, a C++
exception bridge, or a general aggregate/variadic managed-call API. The chapters
explain the contracts those extensions would need. Allocator fault injection,
cross-platform/cross-version distribution, and full runtime concurrency stress
remain outside this test matrix.
