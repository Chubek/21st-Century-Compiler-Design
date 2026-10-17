# FFI, interoperability, and native-extension expansion

## Approved scope and reading map

Eight additional chapters implement the approved outline. Existing source IDs
and previous expansions remain stable; `index.ltx` sets printed numbering.

| Source | Subject | Placement |
| --- | --- | --- |
| 69 | Semantic boundary contracts, conversions, effects, temporal scope | After source 24 |
| 70 | ABI classification, aggregate rollback, libffi plans, variadic promotions | After source 69 |
| 71 | Native layout, wire records, extents, strings, strides, zero-copy contracts | After source 32 |
| 72 | Ownership, roots/handles/pins, generations, close, cycles | After source 33 |
| 73 | Trampolines, synchronous callbacks, reentrancy, quiescence, thread entry | After source 72 |
| 74 | Error channels, unwind boundaries, exception identity, cleanup, cancellation | After source 73 |
| 75 | Embedding, runtime modules, plugins, stable APIs/ABIs, isolated workers | After source 59 |
| 76 | Working C11/libffi core with Lua and CPython integrations | After source 74 |

Supporting integrations connect the new material to calling conventions (24),
runtime representation (32), GC (33), dynamic loading (59), and semantic/effect
IRs (63). Source 32 now distinguishes rooting from address stability explicitly
and uses a named GC cross-reference rather than assuming it is the next chapter.

The expansion adds 16 algorithms, 16 TikZ diagrams, 16 tables, a source-included
tested C listing, and 32 exercises with answers in
`example-code/ffi-labs/solutions.md`. Algorithms, graphs, and tables are also
exported through the existing strict serializer. No word quota was used.

## Implemented mechanisms and model boundaries

`example-code/ffi-labs/` contains:

- A C11 core with checked int32 addition, bounded owned byte buffers, overflow-safe
  extent checks, synchronous callback traversal, and failure-atomic outputs.
- Address-stable libffi call plans for four scalar kinds and at most eight
  arguments. Runtime bindings invoke the checked native addition through libffi.
- Independent native libffi fixtures for mixed-register aggregates, memory-class
  aggregate returns, promoted variadic arguments, executable closures, pointer
  values, and register exhaustion.
- A Lua 5.4 module using full userdata, protected callbacks, `__gc`, and `__close`,
  plus an embedded host registering the same module under protected evaluation.
- A CPython multi-phase module with per-module state, a 3.11 Limited API floor,
  buffer exports, named capsules, preserved callback exceptions, and explicit close.
- A same-platform version/size-negotiated plugin function table and a native
  dynamic-loader client retaining the library until last use.
- Python models for a restricted SysV register profile, ordinary field layout,
  a byte-record format, moving generation-tagged handles, and atomic abstract
  callback-registration transitions.

The native target is Linux x86-64/System V with the documented C/Lua data model.
Managed callbacks are synchronous on the attached calling thread. Native pthread
workers use independent plans and do not enter managed runtimes. The registration
model is not a native concurrent callback registry. The handle model is not an
implemented moving collector. The wire codec is not a worker supervisor.

The chapters explain asynchronous retirement, foreign-thread runtime attachment,
zero-copy ownership, cancellation, C++ exception translation, and process isolation
as design contracts rather than implemented features. Cross-version Stable ABI
distribution, subinterpreter/free-threaded Python configurations, allocator fault
injection, and broader platform coverage remain untested. A Limited API macro and
`abi3` suffix are not claimed as cross-version test evidence.

## Source verification

New prose, examples, algorithms, and exercises are original. No reference extract
was copied into the manuscript or another directory, and `sources/` was not edited.

- The supplied `gc-handbook.txt` is Jones, Hosking, and Moss's **second edition**
  (CRC Press, 2023). Its title/copyright/catalog pages verify authors, title,
  edition, publisher, ISBN, and DOI. Targeted foreign-reference/handle/pinning
  passages informed the conceptual distinction; official runtime APIs provide
  independent concrete examples.
- The existing verified System V AMD64 supplement, draft 0.99.6, supplies the
  ordinary INTEGER/SSE/memory classification rules used here. The text explicitly
  excludes vector extensions and a complete aggregate classifier. C `sizeof`,
  `_Alignof`, `offsetof`, and compiler-generated calls independently check the
  selected installed-platform cases.
- The official libffi manual source was consulted for `ffi_cif`/type-array
  lifetimes, argument-object addresses, widened narrow integral returns,
  `ffi_prep_cif_var`, and closure storage/context lifetime.
- The official Lua 5.4 manual was consulted for protected calls, error objects,
  string lengths, full userdata, finalization, scope close, module loading,
  and embedding. Builds explicitly select `lua5.4` rather than the separately
  installed default Lua ABI.
- Official CPython 3.14 C API pages were consulted for multi-phase initialization,
  per-module state, the Limited API/Stable ABI distinction, capsules, buffer
  release, exceptions, thread attachment, and free-threaded behavior. The current
  thread documentation is `c-api/threads.html`, linked from the split `init.html`.
- The official Itanium C++ exception ABI documents personality routines and
  search/cleanup phases; the case study makes no C++ exception-bridge claim.
- Official Node-API documentation provides the contrasting runtime-supported
  ABI-stable interface and asynchronous/thread-safe function mechanisms. It is
  discussed and cited, not implemented in this lab.

New bibliography keys are `itanium-2026-exceptions`, `jones-2023-garbage`,
`libffi-2026-manual`, `node-2026-api`, and `python-2026-capi`. Existing C11, Lua,
and AMD64 references are reused. Verified URLs are retained in `references.bib`.

## Executable verification

Command:

```sh
PYTHONDONTWRITEBYTECODE=1 python3 -m unittest discover -s example-code/ffi-labs -p 'test*.py' -v
```

All five test methods passed, including the complete compiler/runtime matrix and
four independent model methods. Host dependencies observed: CPython 3.14.7,
Lua 5.4.9, libffi 3.8.0, GCC 16.2.1, and Clang 22.1.8; the host reports
`Py_GIL_DISABLED=0`. GCC and Clang use strict warnings as errors, with normal
and AddressSanitizer/UndefinedBehaviorSanitizer native configurations.

Per native configuration, the fixtures check 49 signed-add boundary pairs,
eight-argument and mixed-bank calls, pointer identity, Pair/Triple arguments and
returns, promoted variadic calls, 101 executable closure invocations, failure
outputs, byte-buffer boundaries, callback early termination/reentrancy, four
workers making 1,000 calls each, and version/size-rejected plugin queries.

The embedded Lua host runs under all four configurations. Ordinary dynamically
loaded Lua and CPython modules run under both compilers; those system interpreter
processes are not sanitizer-instrumented. Scripts check embedded zeros and byte
255, input-copy independence, Python export release, explicit/repeated close,
GC, Lua scope close, nested callbacks, conversion failure, original error identity,
and independent Python state after reimport.

The model suite checks 195 record round trips, every truncated prefix of those
records (7,215 cases), trailing-byte rejection, register-bank rollback, hidden
result-pointer pressure, ordinary field layout, handle relocation/reuse, and
all 16,384 seven-event registration words, checking their legal prefixes.
These finite checks establish only their stated model properties.

All 37 shared support tests passed. Logs are outside the repository:
`/tmp/opencode/ffi-tests.txt` and `/tmp/opencode/ffi-support-tests.txt`.

## Manuscript verification

The eight-chapter draft build passed. Its log identified an overwide ownership
table and error-adapter diagram, which were corrected before the full build.
Visual inspection of all eight new chapters and their 16 diagrams also found
a reentrant edge crossing the callback-adapter node; it was rerouted around the
node and inspected again. The worker-completion algorithm explicitly requires
serialized table access and protected runtime delivery.

The final full XeLaTeX/Biber/makeindex build passed and produced a **580-page**
`index.pdf`. The final log has no undefined references/citations, duplicate labels,
or Biber warnings. No new overfull boxes remain; the earlier approximately
0.68-point overflow persists outside the expansion. LuaLaTeX was not run, and
older pages were not exhaustively visually re-audited.

```sh
FONTCONFIG_FILE=/tmp/opencode/21cc-build/fonts.conf \
LD_LIBRARY_PATH=/tmp/opencode/21cc-build/usr/lib texlua build.lua
python3 support/export-artifacts.py --check --strict
texlua build.lua check
git diff --check
```

Source inventory and whitespace checks passed. Strict exports verify **121
algorithms, 144 TikZ graphs, and 135 Markdown tables** for the complete manuscript.
New citations resolve to bibliography entries, cross-references resolve, and
global labels remain unique. Reference-source git status matches the baseline
captured before the FFI work.

The full-build transcript is `/tmp/opencode/ffi-full-build.txt`; saved final
PDF/logs are `/tmp/opencode/21cc-build/ffi.pdf`, `ffi.log`, and `ffi.blg`.
Rendered review pages are under `/tmp/opencode/21cc-build/ffi-review/` and the
final text extraction is `/tmp/opencode/ffi-final.txt`. Ordinary auxiliaries
were cleaned after archiving; the root `index.pdf` remains available.

No new TODO markers were added. Earlier whole-book citation and verification
items remain documented in `THIRD-REVIEW.md` and the previous expansion records.
No commits or pushes were made.
