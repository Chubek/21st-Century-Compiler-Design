# Third manuscript refinement

Scope: preserve the 38-chapter structure and strengthen the existing exposition,
algorithm contracts, complexity arguments, and worked examples. All 38 chapter
files received targeted edits. Appendix C now explicitly explains the two lattice
order conventions. This is a substantive technical refinement pass, not a claim
that every remaining exercise or production-system attribution has been verified.

## Corrections by chapter

| Chapter | Main refinement |
| --- | --- |
| 1 | Separate analysis soundness, rewrite legality, and fact lifetime as three obligations. |
| 2 | State mathematical-integer semantics, initial bindings, conditional termination, and scope-lookup costs; distinguish redundant from meaning-changing parentheses. |
| 3 | Prove scanner progress by cursor advancement, include comment failure and lookahead bounds, and count lexeme hashing. |
| 4 | Identify the grammar table as an overview; supply comparison parsing, statement cases, and the full-input validation obligation. |
| 5 | Clarify resolution/checking order; add literal and assignment cases to the checker. |
| 6 | Preserve resolved binding identity and explain why effectful expressions would require value capture. |
| 7 | Handle labeled leaders and distinguish unreachable cycles from predecessor-free blocks. |
| 8 | Make the first dominator sweep explicit and correct unreachable-block handling. |
| 9 | Distinguish conservative may answers from witnesses; separate boundaries from empty meets; count visits, bit-vector work, and invalidation. |
| 10 | Define lattice operations by their order; qualify infinite-height iteration; replace the false visit-count exercise with a strict-change proof. |
| 11 | Add branch-sensitive edge transfers, inflationary updates, cycle-covering widening points, and a sound narrowing argument; recompute the interval refinement. |
| 12 | Distinguish unpruned placement from pruning, explain edge uses and missing definitions, and add a primary SSA reference. |
| 13 | Separate termination from a linear rewrite-count claim and account for affected users. |
| 14 | Distinguish unknown input values from solver markers, count product height, and repair the false bottom-initialization exercise. |
| 15 | Name the conservative algorithm accurately and separate its marking cost from dependency construction and aggressive control-flow rewriting. |
| 16 | Replace stale representative reuse with current-holder tracking; retain immutable expression keys; repair cross-block claims, costs, and exercises. |
| 17 | Make invariant iteration explicit; exclude phis; insert before terminators; repair preheaders and partial-redundancy reasoning. |
| 18 | Require reachable graphs, distinguish symbolic counts from unknown counts, and count loop containment work. |
| 19 | State unrolling arithmetic preconditions and distinguish ordered unrolling from parallel execution. |
| 20 | Separate alias relations from conservative answers; specify dynamic load/store dependencies; repair the points-to trace and singleton reasoning. |
| 21 | Make availability iteration and pre-instruction facts explicit; include externally observable memory at exits, must-overwrites, promotion exits, and layout constraints. |
| 22 | Include unknown callees; separate may-read from may-write; qualify cloning precision and delta-propagation cost. |
| 23 | Add a finite expansion policy and distinguish it from a code-size budget. |
| 24 | Establish the outgoing stack position before writing arguments; separate virtual constraints from physical call repair. |
| 25 | Restrict tree optimality to its cost model, charge tile frontier subtrees, and separate tree selection from shared-value handling. |
| 26 | Count fit-search attempts and idle-cycle advances, and use edge delays for readiness. |
| 27 | Qualify the coloring model, preserve precolored nodes, remove the false spill-termination argument, and correct the two-register example and coalescing exercise. |
| 28 | Place copies on the correct edge, including successor-entry placement, and state the data structures needed for linear sequencing. |
| 29 | Specify archive extraction closure and search-order dependence. |
| 30 | Validate program counters, propagate handler errors, and distinguish dispatch fuel from elapsed-time bounds. |
| 31 | Check capacity and depth arithmetic; distinguish decoding, reachable-depth verification, typing, and termination. |
| 32 | State alignment and inheritance-layout assumptions; distinguish GC maps from local-value recovery metadata. |
| 33 | Distinguish insertion/deletion barriers and skip null references during tracing. |
| 34 | Separate desired tier from compilation requests; account for pending work, publication, and counter overflow. |
| 35 | Preserve reconstructed object identity and GC visibility; qualify generic-add continuation and make the arithmetic exercise self-contained. |
| 36 | Tighten legality inputs, count both reduction tiles in the matrix example, and distinguish pair enumeration from dependence-solving cost. |
| 37 | State deterministic same-failure reduction, permitted-edit irreducibility, and quadratic total oracle-call bounds. |
| 38 | State structural-check costs in terms of changing IR sizes and separate expensive semantic validators. |

Later runtime test suggestions now use instructional wording rather than implying
that unimplemented integration tests were run.

## Bibliography

Added four primary references: Kildall (1973), Cousot and Cousot (1977), Cytron et
al. (1991), and Poletto and Sarkar (1999). Author, venue, year, and pagination
metadata were checked through Crossref's DOI records. The full Cousot title was
also checked on the authors' publication page. Five citation TODOs were resolved;
31 remain. No new TODO markers were added.

This verifies the added bibliographic records, not every attribution to the
existing four background references.

## Executable checks

`example-code/refinement_checks.c` adds bounded checks for the revised material:

- Local value numbering: five directed fixtures over all 65,536 byte input pairs,
  plus all 1,728 three-instruction sequences from a 12-instruction catalog over
  64 input pairs. Each check compares every final register with an independent
  interpreter. These cover overwritten representatives, surviving copies,
  self-updates, old expression keys, folding, and commutativity.
- Points-to propagation: compare dynamically registered dependencies with an
  independent full-sweep equation solver under all 120 constraint-priority
  permutations. The indirect load must observe a pointer stored through an alias.
- Matrix tiling: compare every output element with a naive product and count the
  512 multiply-add updates of the 8-by-8, tile-size-four example.

The value-numbering model uses linear tables for readability; it does not test
the chapter's hash-table performance bound. These finite checks do not prove the
general algorithms correct.

Run from the repository root with assertions enabled:

```sh
cc -std=c11 -Wall -Wextra -Wpedantic -Werror \
  example-code/refinement_checks.c -o /tmp/opencode/refinement-checks
/tmp/opencode/refinement-checks
```

All nine executable targets passed with GCC and Clang, both normally and with
address/undefined-behavior sanitizers: expression tests, stack tests, fault lab,
trap lab, demo, observations, prior review checks, new refinement checks, and
visual labs. This is 36 successful compile/run combinations. Records are in
`/tmp/opencode/third-review-tests/transcript.txt`.

## Checked exercise corrections

- **10, height proof:** a chain with at most H elements permits H-1 strict
  changes. Initial scheduling plus notifications gives O(N+E(H-1)) queue visits
  with duplicate-free membership, not H visits per block. The sign example
  changes top to zero to bottom, then has an unchanged visit.
- **14, initialization:** literal assignments make the diamond's final `d=7`
  survive even when nonboundary states start at bottom; `e` remains varying.
  For the loop equation X = meet(7,X), the two fixed points are 7 and bottom.
  Starting at top finds 7; starting at bottom stays bottom.
- **16, holder trace:** with input numbers a=1, b=2, c=3, the sequence in the
  first exercise gives p=4, q=5, then a=2, r=6, and s=5. The last instruction
  copies q. Changing a does not change p's stored value.
- **16, global availability:** while entry computes m*n and operands remain
  unchanged, it dominates the join, so no insertion is needed. Removing that
  entry computation leaves the left-only result; insert on the right path and
  merge results with a phi to remove the join recomputation.
- **20, indirect constraints:** a and b target m, c targets n, the store puts n
  in pts(m), and d targets n. The permutation check verifies propagation of this
  stored content regardless of worklist priority.
- **27, conservative coalescing:** the explicitly specified degree-less-than-k
  test rejects degree three with k=3. A star with three noninterfering leaves
  shows why this rejection does not prove uncolorability.
- **35, recovery arithmetic:** inputs 0 through 599 sum to 179,700 before the
  guard at input 600. Recovery must retain that total, index 600, the already-read
  float, and the before-add resume position.

The Chapter 11 concrete traces were also recomputed for all eleven integer
inputs from -5 through 5: body-entry i lies in [0,4], and exit i=abs(n) lies in
[0,5]. The narrowing derivation joins initial [0,0] with back-edge [1,5].

## Document verification

- `texlua build.lua check`: all expected files present.
- Source audit: 57 TeX files with balanced environments, 687 unique labels,
  438 resolved explicit references, and 223 resolved citation commands using
  eight bibliography entries.
- Final full XeLaTeX/Biber/makeindex build: **293 pages**, with no undefined
  references, undefined citations, duplicate-label warnings, or Biber warnings.
  Makeindex accepted all 322 entries with no warnings or rejected entries.
  The existing 0.67662-point overfull box remains; no new overfull boxes appear.
- Visually inspected PDF pages 113, 143, 172, 238, and 289: abstract interpretation,
  value numbering and its trace table, load elimination, field layout, and the
  expanded bibliography. No clipping or collisions were found. Two final wording
  refinements were then fully rebuilt with the same clean-log result.
- Confirmed the build snapshot matches all 84 current manuscript, support, figure,
  and example files. The PDF is `/tmp/opencode/third-review-build/index.pdf`;
  the final transcript and logs are in the same directory.

Builds use an isolated manuscript copy under `/tmp/opencode/third-review-build`
with the existing host-local font and Biber environment described in `README.md`.
The copy excludes `sources/`; no reference extracts or generated build products
were added to the manuscript tree.

## Remaining work

The remaining citation TODOs, a complete independently checked exercise solution
set, end-to-end implementations of later chapters, and a full rendered-page audit
remain open. LuaLaTeX was not tested in this pass. Historical reports in
`SECOND-REVIEW.md` and `BOOK-PLAN.md` remain historical records.
