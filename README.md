# 21st Century Compiler

An original book about compiler construction, optimization, and runtime systems.
See [BOOK-PLAN.md](BOOK-PLAN.md) for the approved outline, content-driven scope,
current progress, and next writing tasks. Follow [AGENTS.md](AGENTS.md) for all
repository work. Reference extracts in `sources/` are read-only.

**Status:** All 93 chapters and four appendices have drafts. A third refinement
pass strengthens technical contracts, examples, and explanations. Source coverage is complete;
technical validation and primary-source citations are still in progress.
Chapter lengths follow content, with no minimum or maximum word quota.
See [THIRD-REVIEW.md](THIRD-REVIEW.md) for current chapter-by-chapter corrections,
executable checks, and remaining verification work.

The logic and verification expansion adds two logic chapters, six chapters on
static analysis/model checking and compiler validation, and a case study building
a C11 model checker. See [VERIFICATION-EXPANSION.md](VERIFICATION-EXPANSION.md) for
the reading map, source checks, and verification record.
Source IDs remain stable: `draft 48` selects `chapters/ch48.tex`, not printed
chapter 48. `index.ltx` determines reading order.

Algorithms, TikZ diagrams, and Markdown tables are available in
[`pseudo-code/`](pseudo-code/README.md), [`graphs-code/`](graphs-code/README.md),
and [`tables-code/`](tables-code/README.md), respectively.
Regenerate them with `python3 support/export-artifacts.py`; verify synchronization
with `python3 support/export-artifacts.py --check --strict`. The standard-library
exporter requires Python 3.10+; see [its documentation](support/export-artifacts.md).

## Lexing, parsing, and Aurocks

Four additional chapters (`ch49.tex`–`ch52.tex`) develop lexer construction,
predictive grammar design, backtracking/generalized parsing, and a working
Perl-to-C99 parser generator. They follow the introductory parsing chapter.
See [FRONT-END-EXPANSION.md](FRONT-END-EXPANSION.md) for the verification record
and [support/aurocks.md](support/aurocks.md) for the generator contract.

```sh
perl support/aurocks.pl example-code/aurocks/calculator.g > /tmp/opencode/calc.c
cc -std=c99 /tmp/opencode/calc.c -o /tmp/opencode/calc
/tmp/opencode/calc '2+3*(4+1)'
```

## Assembly, linking, and Lua

Eight substantial chapters (`ch53.tex`–`ch60.tex`) develop assembly and linking,
four chapters each. Their C11 case studies embed Lua 5.4: `luaas` preprocesses
assembly with Lua and writes x86-64 ELF64 objects; `luald` uses Lua memory maps
to emit static native executables. The examples include independent GNU-tool
checks and a separate dynamic-loading/TLS experiment.

```sh
sh example-code/lua-toolchain/build.sh /tmp/opencode/lua-tools gcc
PYTHONDONTWRITEBYTECODE=1 python3 example-code/lua-toolchain/test.py
PYTHONDONTWRITEBYTECODE=1 python3 example-code/lua-toolchain/checks.py
```

See [the toolchain documentation](example-code/lua-toolchain/README.md) and
[ASSEMBLY-LINKING-EXPANSION.md](ASSEMBLY-LINKING-EXPANSION.md).

## Compiler architecture and adaptive execution

Eight interleaved chapters (`ch61.tex`–`ch68.tex`) develop incremental compiler
databases, equality saturation, semantic/effect IRs, binary lifting, adaptive
interpreters and staging, speculative state transfer, continuous compilation,
and profile-guided/learned/spatial optimization. Supporting sections connect
them to nine existing chapters.

```sh
PYTHONDONTWRITEBYTECODE=1 python3 -m unittest discover -s example-code/architecture-labs -v
```

See [the lab contracts](example-code/architecture-labs/README.md),
[32 exercise solutions](example-code/architecture-labs/solutions.md), and
[ARCHITECTURE-EXPANSION.md](ARCHITECTURE-EXPANSION.md) for the reading map,
source checks, executable evidence, and implementation boundaries.

## FFI, interoperability, and native extensions

Eight interleaved chapters (`ch69.tex`–`ch76.tex`) develop semantic contracts,
ABI classification, marshaling, ownership and GC, callbacks and threads,
cross-language errors, extension models, and a working C11/libffi case study
with Lua 5.4 and CPython bindings.

```sh
PYTHONDONTWRITEBYTECODE=1 python3 -m unittest discover -s example-code/ffi-labs -p 'test*.py' -v
```

See [the lab contracts](example-code/ffi-labs/README.md),
[32 exercise solutions](example-code/ffi-labs/solutions.md), and
[FFI-EXPANSION.md](FFI-EXPANSION.md) for supported mechanisms, source checks,
and verification evidence.

## Strings, rewriting, and functional languages

Seventeen substantial new chapters (`ch77.tex`–`ch93.tex`) cover strings and FSTs
(four chapters), rewrite systems and compiler graph transformations (eight),
and functional-language implementation (five). They include an executable
FST-based spellchecker, rewriting laboratories, and a typed functional core with
strict source/compiled execution and an alternate call-by-need evaluator.

```sh
PYTHONDONTWRITEBYTECODE=1 python3 -m unittest discover -s example-code/string-labs -v
PYTHONDONTWRITEBYTECODE=1 python3 -m unittest discover -s example-code/rewriting-labs -v
PYTHONDONTWRITEBYTECODE=1 python3 -m unittest discover -s example-code/functional-labs -v
python3 example-code/string-labs/strings.py example-code/string-labs/dictionary.tsv ct --radius 1
python3 example-code/functional-labs/fun.py example-code/functional-labs/sum.fun
```

See [STRINGS-REWRITING-FUNCTIONAL.md](STRINGS-REWRITING-FUNCTIONAL.md) for the
reading map, source verification, finite test evidence, and implementation limits.
The three lab directories contain their own contracts and all 68 exercise answers.

## Implementation companion: PebbleIR

**PebbleIR: Design, Development and Dogfooding** is a separate 16-chapter
companion at [`book/pamphlet.ltx`](book/pamphlet.ltx), with a working C11 IR
implementation, expression compiler, verifier, optimizer, interpreters, and tests.

```sh
texlua book/build.lua check
texlua book/build.lua
```

See [`book/README.md`](book/README.md) for the reading map and build instructions,
and [`example-code/pebbleir/README.md`](example-code/pebbleir/README.md) for runnable
examples. The output is `book/pamphlet.pdf`.

## Build the main book

Requires XeLaTeX (or LuaLaTeX), Biber, makeindex, and the fonts Source Serif 4,
Source Sans 3, and JetBrains Mono. `minted` uses its Pygments helper
(`latexminted` for minted 3). `minted` and `stmaryrd` are standard TeX Live
packages: the former supplies the requested code highlighting, and the latter
supplies semantic evaluation brackets.

**Build trusted sources only.** The driver enables TeX shell escape for code
highlighting. It adds `support/` to `TEXINPUTS` while retaining an existing search
path and the default TeX search path.

```sh
texlua build.lua fonts
texlua build.lua draftfull 1
```

`draftfull` includes the preface and selected chapters and runs Biber, makeindex,
and the resolving engine passes. `draft 1` is a faster two-engine-pass preview
without bibliography/index regeneration. Both produce `index.pdf`; a partial
build is not a complete-book build.

```sh
texlua build.lua check
texlua build.lua
```

All expected source files are now present. These commands check source coverage
and build the complete manuscript; neither substitutes for technical review.

Use `ENGINE=lualatex` to select LuaLaTeX. `texlua build.lua clean` removes the
usual auxiliary files, and `distclean` also removes `index.pdf`. Minted caches
are ignored by version control; do not include them in manuscript changes.

### This host's build dependencies

The initial environment lacks Source Serif 4 and Source Sans 3, and its Biber
binary needs `libcrypt.so.1`. Verification uses temporary, build-local copies
under `/tmp/opencode/21cc-build`, without changing system fonts or libraries:

- Required font files from Adobe's official `source-serif` and `source-sans`
  repositories, exposed through a temporary Fontconfig configuration.
- The compatibility library from Arch's official `libxcrypt-compat` package,
  exposed through `LD_LIBRARY_PATH`.

While those temporary files remain available, the verification environment is:

```sh
FONTCONFIG_FILE=/tmp/opencode/21cc-build/fonts.conf \
LD_LIBRARY_PATH=/tmp/opencode/21cc-build/usr/lib \
texlua build.lua draftfull 1
```

For a lasting setup, install the documented fonts and the Biber runtime
dependency through the host's normal package process. Temporary paths are not
repository dependencies and are not downloaded automatically by the build.

## Run the examples

The [Chapter 1 example README](example-code/ch01/README.md) gives C11 build and
test commands. Executables belong outside the manuscript directory. The exercise
checks are recorded separately in `example-code/ch01/SOLUTIONS.md`.

The [visual laboratories](example-code/visual-labs/README.md) provide tested C11
listings for decimal scanning, parallel copies, stack-depth checks, and byte
alignment. The [figure catalog](figures/README.md) maps the new diagrams to their
chapters and explains what each one illustrates.

The [third review](THIRD-REVIEW.md#executable-checks) documents additional C11
differential checks for value numbering, indirect points-to propagation, and
tiled matrix arithmetic in `example-code/refinement_checks.c`.

## Audit chapter length

```sh
texcount -inc -opt=support/texcount.opts chapters/ch01.tex
```

The `-inc` option includes chapter pieces loaded with `input`.
The shared options count text, headings, and captions, not equations,
pseudocode, minted file arguments, or index entries. Bibliography entries
are not part of chapter prose.
