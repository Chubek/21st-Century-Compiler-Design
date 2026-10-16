# PebbleIR: Design, Development and Dogfooding

A 16-chapter implementation companion to *21st Century Compiler*.
The master source is `book/pamphlet.ltx`; chapters live in `book/chapters/`.
The companion shares the main book's class, pseudocode style, and bibliography,
but produces its own PDF, contents, chapter numbers, bibliography, and index.

From the repository root:

```sh
texlua book/build.lua check
texlua book/build.lua
```

The output is `book/pamphlet.pdf`. Use `ENGINE=lualatex` to select LuaLaTeX.
The driver requires a POSIX shell and the same fonts, Biber, makeindex, and minted
setup as the main book; see the root README for this host's temporary font and
library environment. It compiles trusted sources with shell escape.

The [C11 implementation](../example-code/pebbleir/README.md) builds and runs
separately from LaTeX. [SOLUTIONS.md](SOLUTIONS.md) contains worked answers to all
32 chapter exercises. [VERIFICATION.md](VERIFICATION.md) records checks actually
performed and the implementation boundary.

## Reading map

1. Requirements and the first client
2. Semantic state and observations
3. Function-local IDs and object ownership
4. Types and instruction contracts
5. Storage, revisions, and failure atomicity
6. Builders and backpatching
7. Text snapshots and round trips
8. Representation verification
9. CFGs and dominance
10. Direct SSA and the general conversion extension
11. Transactional mutation
12. The reference interpreter
13. Pass boundaries and instrumentation
14. Constant folding and demand-based deletion
15. Phi lowering and the register VM
16. Compiler clients, differential checks, reduction, and measurement
