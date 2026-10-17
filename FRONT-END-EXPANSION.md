# Lexing, parsing, and the Aurocks case study

## Approved scope and reading order

The four additional chapters include the case study, with C retained as the
output language of the supplied Perl generator. They follow the introductory
lexing and parsing chapters. Existing source filenames remain stable; draft
arguments select source IDs, not printed chapter numbers.

| Source | Printed chapter | Subject |
| --- | --- | --- |
| `chapters/ch49.tex` | 5 | Regular languages and lexer construction |
| `chapters/ch50.tex` | 6 | Predictive parsing and grammar design |
| `chapters/ch51.tex` | 7 | Backtracking and generalized parsing |
| `chapters/ch52.tex` | 8 | Case study: building Aurocks |

The expansion adds 14 pseudocode algorithms, 12 original TikZ diagrams, seven
computed tables, three source-backed listings, and 16 exercises with checked
answers in `example-code/aurocks/SOLUTIONS.md`. It develops NFA fragments,
determinization, output-preserving minimization, maximal-munch rollback,
nullable/FIRST/FOLLOW, LL tables, AST associativity, recovery, continuations,
semantic commitment, chart recognition, and the implementation's contracts.
No word quota was applied.

## Sources and attribution

- `sources/grune-and-jacobs-parsing.txt`: Grune and Jacobs, *Parsing Techniques:
  A Practical Guide*, second edition, Springer, 2008. Title, edition, date,
  and ISBN checked against the supplied front matter. Consulted for lexical
  construction, exhaustive recursive descent, predictive methods, and charts.
- `sources/hopcroft-automata.txt`: Hopcroft, **Motwani**, and Ullman,
  *Introduction to Automata Theory, Languages, and Computation*, third edition,
  2007. All three authors and ISBN checked; used for regular/context-free
  language foundations, subset construction, and equivalence arguments.
- `sources/gll-parsing.txt`: van Binsbergen, Scott, and Johnstone, *GLL Parsing
  with Flexible Combinators*. The supplied preprint has placeholder publication
  metadata. Published details verified through Crossref: SLE 2018, pp. 16–28,
  DOI `10.1145/3276604.3276618`.

The bibliography has corresponding `grune-2008-parsing`,
`hopcroft-2007-automata`, and `vanbinsbergen-2018-gll` entries. The new prose,
diagrams, exercises, and worked computations are original presentations.
Reference extracts were not modified. No new citation or technical TODOs were
introduced; earlier textbook-wide review obligations remain.

## Working generator

`support/aurocks.pl` retains and validates the supplied regex AST/NFA machinery.
`support/aurocks-emit.pl` emits grammar-specific tables and actions, using the
readable C template `support/aurocks-runtime.c`. Output is standalone C99.
`support/aurocks.md` documents syntax, limits, semantic substitutions, ownership,
and migration from the earlier interface.

Implemented properties include:

- Continuation-based backtracking across caller boundaries, with the first
  complete parse in production order selected.
- Semantic actions deferred until complete recognition; rejected branches do
  not run actions or allocate semantic terminal copies.
- Grammar tokenization respecting strings/comments and nested action braces;
  rejection of undefined symbols, unsupported directives, nullable terminals,
  and direct/indirect/nullable-prefix left-recursive cycles.
- Atomic longest-prefix terminal matching, explicit input lengths, byte spans,
  and a documented ASCII regex subset.
- Separate success, syntax, resource-limit, allocation-failure, and invalid-API
  statuses; result-owned allocation and cleanup.
- Deterministic output, capacities, depth/work limits, and no partial emitted C
  for invalid grammars.

The regression grammar accepts both `ab` and `abb`, executing two selected
production actions, and rejects `abx` with zero actions. The calculator evaluates
`2+3*(4+1)` to 17 within its documented bounded arithmetic domain.

This is a depth-first backtracking implementation, not GLR, GLL, PEG packrat,
or a forest enumerator. Runtime budget exhaustion is inconclusive. Regex
terminals do not backtrack to shorter endpoints; semantic actions are not
recognition predicates and are outside the search budget. The comparisons and
chart pseudocode in the book explicitly distinguish these contracts.

## Executed verification

```sh
PYTHONDONTWRITEBYTECODE=1 python3 -m unittest discover -s support -p 'test_*.py' -v
python3 support/export-artifacts.py --strict
python3 support/export-artifacts.py --check --strict
texlua build.lua check
FONTCONFIG_FILE=/tmp/opencode/21cc-build/fonts.conf \
LD_LIBRARY_PATH=/tmp/opencode/21cc-build/usr/lib texlua build.lua
```

- **37 tests passed:** 11 generator tests, four front-end computation tests,
  and the existing 22 serialization tests.
- Generated-parser execution uses four configurations: GCC and Clang, each
  with strict C99 warnings and again with ASan/UBSan and leak detection.
- Each configuration checks all 511 binary words through length eight against
  an independent recognizer for `a^n b^n`, and all 5,461 words over `{a,b,c,d}`
  through length six against an independent oracle for `a(b|c)*d`.
- Additional execution covers complete-input backtracking, action commitment,
  exact/insufficient budgets, malformed grammars, nested action braces,
  character classes, counted repetition, embedded zero bytes, literal escaping,
  C99 trigraph protection, allocation failure, and cleanup.
- Independent Python checks recompute the five subset states and four minimized
  states, nullable/FIRST/FOLLOW and LL conflicts; predictive recognition is
  compared on 21,845 token sequences. Chart tests cover left recursion and
  nullable completion, including 255 binary strings through length seven.
- Strict exports are synchronized: **73 algorithms, 88 TikZ graphs, 82 tables**
  across the main book. The exporter now renders the chart's dot marker while
  retaining exact original LaTeX in its provenance section.
- Draft and full XeLaTeX/Biber/makeindex builds passed. Final main PDF:
  **431 pages**, with no undefined references/citations, duplicate-label
  warnings, or Biber warnings. Only the pre-existing approximately 0.68 pt
  overfull box remains; no new overfull boxes.
- All 12 new diagram pages were visually sampled. The predictive-choice diagram
  had overlapping labels; these were corrected and the final page rechecked.
  The Perl and C implementation listings were also inspected in the final PDF.

The PDF is `index.pdf`. A build snapshot, log, Biber log, full command output,
and rendered review pages are retained externally under
`/tmp/opencode/21cc-build/`; test output is
`/tmp/opencode/aurocks-tests-final.txt`. Build auxiliaries were cleaned after
saving the verification evidence. No commit or push was made.

## Remaining limits

The bounded tests and implementation invariants do not establish correctness for
every grammar/input or arbitrary user C actions. LuaLaTeX and a complete
page-by-page visual audit were not performed. Earlier citation gaps and older
chapters' implementation/exercise review remain recorded in the existing review
files. Vendored Aurocks copies under `PebbleIR/third_party/` remain separate.
