# Second manuscript review

Scope: all 38 chapter files, their Chapter 1 input files, and all three
appendices. Length is content-driven; there is no word quota.

This pass checks source structure across the entire manuscript and reviews
algorithm contracts and worked computations. It is not an independent proof
of every algorithm, a completed solution manual, or a primary-source audit.
The previous claim that a successful build meant the book was finished was
too strong. All chapters exist, but substantial validation remains.

## Coverage and corrections

### Follow-up: figures and executable listings

Added 11 captioned, referenced TikZ diagrams across Chapters 3, 9, 12, 19,
26, 28, 31, 32, 33, and 35, and Appendix C; see `figures/README.md`.
Four C11 listings are included directly from `example-code/visual-labs/`.
Their tests pass strict GCC and Clang builds, both normally and with address
and undefined-behavior sanitizers. These are component checks, not complete
compiler implementations. No citation TODOs were added.

Corrected an additional load-latency claim in Chapter 26 and the expression
instruction count and verifier contract in Chapter 31. Redrew existing lexer,
loop-unrolling, and critical-edge figures after visual inspection exposed
overlapping labels or misleading graph edges.

The full XeLaTeX/Biber/makeindex build now produces 285 pages. Source checks
find 687 unique labels and 436 resolved explicit references, including all new
figures and listings. There are no undefined references, undefined citations,
or duplicate-label warnings; the existing 0.68 pt overfull box remains.
All new diagram pages and listing pages were visually inspected, as were the
three repaired older diagrams. This is not a full-book visual inspection.
LuaLaTeX was not tested. The prior outstanding technical-review obligations
remain. Build records and the PDF are retained in `/tmp/opencode/21cc-build/`.

| Chapter | Review focus and outcome |
| --- | --- |
| 1 | Trap propagation through addition/multiplication; unrolling preserves dynamic effects rather than duplicating the trace. |
| 2 | Interpreter dispatch now includes assignment and lexical binding. |
| 3 | Scanner recognizes comments at their opening delimiter and terminates on a lexical error. |
| 4 | Parse trace counts all seven AST allocations, including leaves. |
| 5 | Separate hashed scope frames do not imply constant-time lookup across scopes. |
| 6 | Literal materialization, temporary count, and label count now agree with the lowering rule. |
| 7 | A return also ends a basic block and starts a leader after it. |
| 8 | Dominator algorithm explicitly restricts its graph to reachable blocks; layout repaired. |
| 9 | Reaching-definition diamond and worklist structure inspected; whole-program complexity needs further review. |
| 10 | Lattice order and initialization made consistent; subset-lattice height counts chain elements. |
| 11 | Galois connection stated formally; mathematical versus fixed-width arithmetic distinguished; loop trips separated from header visits. |
| 12 | Frontier definition, table, placement trace, exit renaming, semi-pruned SSA, and complexity corrected. |
| 13 | Rewritten instruction is requeued so the stated normal-form guarantee is possible. |
| 14 | Meet absorption distinguished from a subsequent assignment; removed the claim that multiple definitions necessarily defeat flow-insensitive analysis. |
| 15 | Conservative deletion retains control flow and possibly diverging/trapping operations; reaching definitions and liveness trace corrected. Aggressive branch deletion remains a separate obligation. |
| 16 | Updating variable value numbers distinguished from deleting old expression keys; unsupported student-compiler anecdote removed. Representative availability still needs a fuller implementation. |
| 17 | Hoisting restricted to pure total SSA operations with available operands, visited before their users. |
| 18 | Self-loop predecessor walk fixed; preheader execution distinguished from body execution; dominator cost includes set operations. |
| 19 | Final failed header tests included in dynamic counts, including cleanup. |
| 20 | Intersecting abstract points-to sets indicate may-alias, not a concrete alias witness. Dynamic load/store constraints need a more explicit solver before implementation. |
| 21 | Diamond now branches explicitly; load facts retain values and dominance requirements; transfer respects instruction order. Removed universal memory-performance claims. |
| 22 | Bottom-up may-modify trace inspected; unknown callees and implementation-specific propagation cost need further validation. |
| 23 | Constant stride does not establish base alignment. |
| 24 | ABI register shuffling uses parallel-copy semantics. The target is illustrative, not a verified real ABI. |
| 25 | Displacement-only tile cost corrected from four to three under the stated unit costs. |
| 26 | Producer latency, critical-path heights, issue times, completion time, and dual-issue comparison corrected. |
| 27 | Interference edges and split-live-range reload placement corrected for read-before-write instructions. |
| 28 | Malformed listing terminator repaired; invalid phi spelling and incorrect swap sequence replaced; two live results cannot share one register. |
| 29 | Relocation arithmetic inspected; actual encodings and ABI-specific addend conventions still need primary references. |
| 30 | Four-instruction assignment and remaining fuel checked by hand; dispatch-loop failure interfaces need an implementation. |
| 31 | Underflow checks operand arity, not just net stack change; sum has eight dispatches including halt. |
| 32 | Field offsets and padding checked by hand; inheritance/tail-padding policy remains runtime-specific. |
| 33 | Root deduplication, stable-heap precondition, gray-set trace, initialization cost, and cascading reference-count reclamation clarified. |
| 34 | Tier threshold arithmetic inspected; thresholds and profiles are illustrative rather than measured policy. |
| 35 | Per-input tag check stays inside the loop; recovery arithmetic inspected. A concrete dynamic language is still needed for a full end-to-end example. |
| 36 | Recognized reductions considered before rejecting loop-carried dependence; unsupported cache-miss bounds removed. |
| 37 | Reducer restarts after successful deletion to justify single-deletion irreducibility. |
| 38 | Dead-code example no longer claims folding is needed to detect an unused definition; pipeline checks promise detection only within their coverage. |
| Appendix A | Stack-verification wording corrected; pure-language reference checked against the opening code. |
| Appendix B | Sanitizer limitations, source inventory, and removal of word quota corrected. |
| Appendix C | Forward/backward transfer direction and analysis-dependent notation clarified. |

## Executable checks

`example-code/review_checks.c` independently recomputes Chapter 12 dominators
and frontiers from graph edges. It also checks the Chapter 18 self-loop,
all 65,536 byte-valued swaps from Chapter 28, Chapter 31 operand arity,
Chapter 19 loop counts, Chapter 26 schedule constraints, and Chapter 27
allocation on 1,024 input pairs. These are bounded example checks, not
proofs of the general algorithms.

Build and run with assertions enabled:

```sh
cc -std=c11 -Wall -Wextra -Wpedantic -Werror example-code/review_checks.c -o /tmp/opencode/review-checks
/tmp/opencode/review-checks
```

Verification uses GCC and Clang, then each compiler with
`-fsanitize=address,undefined`. Document verification uses the full
XeLaTeX/Biber/makeindex build, not only a chapter selection. Source checks
include environment balance, unique labels, and resolution of explicit
references, including labels supplied as listing options.

Final verification for this pass: the full XeLaTeX/Biber/makeindex build
succeeded at 279 pages. All 44 chapter-piece and appendix source files passed
the structural audit: 672 unique labels and 419 resolved explicit references.
No undefined references, undefined citations, or duplicate labels were reported.
One 0.68 pt overfull box remains. GCC and Clang passed the new executable
checks both normally and with address/undefined-behavior sanitizers. The
repaired Chapter 28 algorithm and listing page was also inspected as an image;
this is not a claim of visual inspection of all 279 pages.
The PDF is retained outside the source tree at
`/tmp/opencode/21cc-build/complete-draft.pdf`.

## Remaining review work

- Resolve the 36 citation TODOs with verified primary sources.
- Implement and test the later chapter algorithms; proposed test scenarios
  in those chapters are not reports of executed test suites.
- Independently check every exercise and record answers.
- Review general complexity bounds against their actual data structures.
- Review every rendered diagram and table; successful typesetting does not
  establish that every drawing expresses the intended graph.
- Test LuaLaTeX separately if that engine is part of the release promise.
