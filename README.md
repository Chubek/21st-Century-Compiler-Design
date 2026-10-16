# 21st Century Compiler

An original book about compiler construction, optimization, and runtime systems.
See [BOOK-PLAN.md](BOOK-PLAN.md) for the approved outline, content-driven scope,
current progress, and next writing tasks. Follow [AGENTS.md](AGENTS.md) for all
repository work. Reference extracts in `sources/` are read-only.

**Status:** All 38 chapters and three appendices have drafts. A third refinement
pass strengthens technical contracts, examples, and explanations. Source coverage is complete;
technical validation and primary-source citations are still in progress.
Chapter lengths follow content, with no minimum or maximum word quota.
See [THIRD-REVIEW.md](THIRD-REVIEW.md) for current chapter-by-chapter corrections,
executable checks, and remaining verification work.

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
