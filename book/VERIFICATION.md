# PebbleIR companion verification

## Delivered material

- `book/pamphlet.ltx`: independent master document with the requested title.
- Sixteen substantive chapters, each with a worked example, an algorithm,
  implementation discussion, two exercises, and further reading.
- Thirty-two worked exercise answers in `book/SOLUTIONS.md`.
- Three original TikZ figures, three tables, one textual IR listing, and three
  C listings included directly from the executable implementation.
- `book/build.lua`: independent source check and full XeLaTeX/Biber/makeindex
  build, sharing the main book's typography and bibliography.
- A reference in the main book's preface and build/run links in the root README.

## Executable implementation

The C11 implementation in `example-code/pebbleir/` includes bounded builders,
stable value IDs, typed byte arithmetic, SSA phis, CFG/dominance verification,
canonical printing and transactional parsing, an expression compiler,
transactional use replacement, constant folding, demand-based DCE, an IR
interpreter, and lowering to a separate register VM with parallel edge copies.

The reference intentionally uses one-function ownership, eight blocks, 128 value
slots, two-input phis, and no memory or call operations. Mutable-variable-to-SSA
conversion, a module layer, SCCP, a reducer, and physical allocation are explained
as extensions rather than represented as implemented features.

## Executed checks

Both `test_pebbleir.c` and `demo.c` were compiled with GCC and Clang using C11,
`-Wall -Wextra -Wpedantic -Werror -O1 -g`, then built and run again with
`-fsanitize=address,undefined -fno-omit-frame-pointer`. All eight compile/run
combinations passed. Assertions remained enabled.

The suite checks:

- All 256 inputs for diamond, summation, and parallel-swap fixtures against
  direct oracles through original/optimized IR and original/optimized VM.
- 128 generated depth-bounded source trees on all 256 inputs: 32,768
  program/input pairs checked against a separate tree evaluator, both IR forms,
  and the optimized VM.
- Print/parse/print equality for the graph fixtures and their optimized forms,
  plus the running expression's optimized snapshot.
- Invalid dominance, phi coverage, condition types, constants, targets, owner
  blocks, and unreachable blocks.
- Failed-replacement, parser, capacity, and saturated-revision rollback.
- IR loop budget eight versus nine for input three; VM budget five versus six
  for the running expression; malformed VM register rejection.
- Precedence and constant-only examples producing 14, 20, and 45, each lowered
  to two VM instructions after optimization.
- The running expression's instruction count changing from eight to six and
  the demo reporting `x=9 result=128 vm-dispatches=6`.

These are finite checks, not exhaustive proofs over graphs or tree shapes.
No wall-clock performance result is claimed.
Test transcripts and executables are outside the source tree under
`/tmp/opencode/pebbleir-tests/`.

## Document checks

- `texlua book/build.lua check`: all sixteen chapters and required implementation
  and support sources present.
- Companion source audit: 164 unique labels, 28 resolved explicit references,
  25 resolved citation commands, balanced environments, and 32 exercise labels
  each matched by exactly one solution. Labels do not collide with the main book.
- Full companion XeLaTeX/Biber/makeindex build: **49 pages**, no undefined
  references or citations, duplicate labels, LaTeX warnings, or overfull boxes.
  Makeindex accepted ten index entries with no rejected entries or warnings.
- Visually inspected companion PDF pages 1, 17, 24, 43, 46, and 48, covering the
  title, both implementation fragments, a lowering diagram, the demo listing,
  and bibliography. No clipping or diagram collisions were found. This is not
  an all-page visual audit.
- Full main-book build after adding its companion reference: **293 pages**,
  no undefined references or citations or duplicate-label warnings. The existing
  0.67662-point overfull box remains; no new overfull boxes were introduced.

Builds used an isolated source copy with the font and Biber library environment
already documented in the root README. The companion PDF is retained at
`/tmp/opencode/pebbleir-build/book/pamphlet.pdf`; the updated main PDF is
`/tmp/opencode/pebbleir-build/index.pdf`. Logs and build transcripts are alongside
them. Normal in-tree use of the driver writes `book/pamphlet.pdf`, which is
ignored by version control.

LuaLaTeX and a full visual audit were not run. No new bibliography records,
packages, fonts, or TODO markers were needed. No changes were made to `sources/`
or the existing third-party libraries.
