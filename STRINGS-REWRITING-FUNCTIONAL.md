# Strings, rewriting, and functional-language expansion

## Approved scope

The maintainer approved the 17-chapter outline and explicitly confirmed that the
spellchecker should be **FST-based**. Sources 77–93 are new chapters, not revisions
counted toward an expansion allocation. Existing filenames and previous
expansions remain intact. `index.ltx` controls printed numbering.

| Source | Chapter | Placement |
| --- | --- | --- |
| 77 | String Representations, Unicode, and String Operations | After lexing overview |
| 78 | Exact Matching, Dictionaries, and String Indexes | After 77 |
| 79 | Finite-State Transducers and Approximate Matching | After 78 |
| 80 | Case Study: Building an FST-Based Spellchecker | After 79 |
| 81 | Abstract Reduction and String Rewriting | After SSA/incremental databases |
| 82 | Terms, Substitutions, Matching, and Unification | After 81 |
| 83 | Termination and Reduction Orderings | After 82 |
| 84 | Confluence, Critical Pairs, and Completion | After 83 |
| 85 | Rewrite Strategies and Efficient Rule Engines | After 84 |
| 86 | Term Graph Rewriting, Sharing, and Cycles | After 85 |
| 87 | Graph Transformation and Compiler IR Rewriting | After 86 |
| 88 | Verified Rewrite-Based Optimization and Instruction Selection | After 87 |
| 89 | Functional Core Languages and Evaluation Semantics | After type theory |
| 90 | Type Inference, Algebraic Data Types, and Pattern Compilation | After 89 |
| 91 | Closures, Lambda Lifting, and Combinators | After 90 |
| 92 | Lazy Evaluation, Graph Reduction, and Runtime Systems | After 91 |
| 93 | Case Study: Building and Optimizing a Functional-Language Implementation | After 92 |

Supporting passages connect lexing (03), instruction selection (25), GC (33),
type theory (39), and equality saturation (62) to the new material. The existing
type-theory text now points to the implemented inference extension instead of
deferring it to an unspecified future session.

Every new chapter contains explicit notation/contracts, original worked examples,
two algorithms, two TikZ diagrams, two tables, and four exercises with worked
answers. Total new exports: **34 algorithms, 34 diagrams, 34 tables**. Total
new solved exercises: **68**. Chapter length follows explanation rather than
a word quota; the eight rewrite chapters include additional proof, substitution,
conditional-rule, certificate, update, and optimization discussions.

## Supplied literature and independent checks

No extract was copied into the manuscript, and no file under `sources/` was
modified. Reference-source git status is checked against the baseline saved in
`/tmp/opencode/strings-rewriting-before.txt`.

- `algorithms-on-strings.txt` identifies Crochemore, Hancart, and Lecroq,
  *Algorithms on Strings*, Cambridge University Press, 2007, ISBN
  978-0-521-84899-2. Title/copyright pages verify the entry; targeted border and
  failure-function passages were consulted. The examples here are newly computed.
- `advanced-topics-in-trs-term-rewriting.txt` identifies Enno Ohlebusch,
  *Advanced Topics in Term Rewriting*, Springer, 2002. Its front matter supplies
  the DOI. Targeted reduction/confluence material checks the termination premise
  in the local-confluence theorem; critical overlaps and joinability are stated
  with finite first-order restrictions, not generalized to arbitrary IR graphs.
- `orderings-for-trs-term-rewriting.txt` is Dershowitz's 1982 article in
  *Theoretical Computer Science* 17, 279–301. The article header verifies its
  metadata. Simplification/path-ordering discussion is kept distinct from the
  executed size/weight examples; there is no claimed general termination prover.
- `term-graph-rewriting-trs.txt` identifies the 1987 Barendregt/van Eekelen/
  Glauert/Kennaway/Plasmeijer/Sleep paper. Title/proceedings metadata and DOI
  were cross-checked through Crossref. Targeted correspondence passages motivate
  the explicit restrictions on graph/term simulation and cyclic read-back.
- `optimal-impl-of-funlangs.txt` identifies Asperti and Guerrini and provides
  material on advanced sharing. Cambridge's book-review metadata, independently
  retrieved through Crossref, verifies the 1998 Cambridge University Press book;
  the review DOI is not misrepresented as the book's DOI. The chapter distinguishes
  ordinary thunk sharing from optimal reduction and makes no claim to implementing
  that book's reducer.
- `combinators-and-funlangs.txt` identifies the 1986 edited Springer proceedings,
  not a single-author implementation manual. Its available contextual material
  distinguishes environment machines and combinator reduction. New combinator
  examples are derived independently rather than copied from the proceedings.
- `ctrlflow-analysis-of-funlangs.txt` identifies Olin Shivers's 1991 CMU thesis,
  CMU-CS-91-145. Title page and abstract verify its identity and subject. The
  higher-order analysis discussion does not claim that analysis is implemented.
- `algebraic-theory-of-graph-reduction.txt` concerns graph-class recognition and
  bounded treewidth, not just functional evaluation. It was not treated as a
  generic programming-language graph reducer merely because its filename says
  graph reduction.
- `spj-impl-of-funlangs.txt` and `flexible-pattern-matching-in-strings.txt` contain
  only form-feed characters. Their contents were not inferred or attributed.
  The STG paper was instead obtained from the author's official Microsoft
  Research publication/PDF, which verifies its author, title, and 1992 date.
- `trs-term-rewriting-and-applications.txt` is the RTA 2007 proceedings edited by
  Franz Baader, not a general introductory textbook. Its identity was checked
  before choosing the more suitable reference for foundational exposition.

Official Unicode normalization and segmentation annexes were independently
consulted. The code uses the installed Python Unicode database, observed as
**16.0.0**; it does not claim conformance to a newer database just because the
current web annex has a newer version. Persisted dictionary indexes should record
their actual database and key policy.

Mohri's author publication index and linked *Weighted Automata Algorithms* PDF
verify the 2009 handbook chapter and its metadata. They independently check
weighted composition, epsilon handling, semiring distinctions, determinization
conditions, and shortest-distance principles. The general intermediate-label
composition algorithm in the text explicitly excludes epsilon; the executed
spelling composition is a specialized fixed-input/trie construction.

Crossref verifies Milner's 1978 polymorphism paper and the title, year, publisher,
and DOI of *Fundamentals of Algebraic Graph Transformation*. Authors were not
invented for the latter record when the available metadata omitted them.
Existing Appel, GC-handbook, and equality-saturation entries are reused.

## Implementations and boundaries

### String/FST laboratory

`example-code/string-labs/` implements prefix-function exact matching, Unicode
canonical keys, and a weighted edit-FST composed lazily with a trie identity FST.
Candidate distance is minimum path cost; insertion/deletion/substitution cost
one and copy costs zero. Frequencies rank only equal-distance candidates.
The API completes the within-radius search before ranking/truncation and raises
explicit incomplete status on state-budget exhaustion. CLI failure prints no
successful result JSON.

Limits: normalized code points, not graphemes; radius 0–3; word/query extents
at most 128 before and after normalization; no primitive transposition, generic
FST library, document tokenizer, morphology, context model, or corpus-quality
claim. The dictionary is an original tiny demonstration. State expansion is not
a total memory/time bound. README documents dictionary-size and prefix-storage
limitations.

### Rewriting laboratory

`example-code/rewriting-labs/` implements finite first-order matching/unification,
occurs checks, immutable positional replacement, bounded strategy selection,
certificate replay, critical-pair generation with renamed-apart variables,
string normalization, finite graph read-back/shared update, restricted dangling
checks, and a tiny target-tree cost fixture.

Matcher instantiation is simultaneous and distinct from the unifier's transitive
substitution resolution; tests cover overlapping rule/subject variable IDs.
Graph fixtures use trusted finite valid heaps and do not implement GC or general
graph matching. Completion, recursive path-order checking, indexed engines,
complete DPO construction, SSA transactions, and parallel graph rewriting are
explained, not misreported as implemented tools. The target selector's costs
are abstract and its simple recursive code is not credited with the complexity
bound of a memoized production selector.

### Functional laboratory

`example-code/functional-labs/` implements validated S-expression syntax, finite
type inference with let schemes and occurs checks, monomorphic recursive uses,
strict lexical evaluation, a structured stack-code compiler/VM, safe local
optimization, and alternate call-by-need evaluation with thunk updates.
Nil/Cons lists and their exhaustive two-arm case are implemented; arbitrary
user-defined ADTs and general pattern-matrix compilation are discussed only.

The VM is compiled structured code interpreted by Python, not native code.
It uses recursive host execution and does not promise proper tail calls.
The lazy machine is an explicitly different evaluation semantics, not a claim
that strict evaluation can be changed to lazy evaluation without observing
termination differences. Python supplies allocation/GC and arbitrary-precision
integers. The lab does not implement a native collector, closure converter,
lambda lifter, STG backend, higher-order CFA, or optimal graph reducer.

## Verification record

Commands, run with no third-party Python packages:

```sh
PYTHONDONTWRITEBYTECODE=1 python3 -m unittest discover -s example-code/string-labs -v
PYTHONDONTWRITEBYTECODE=1 python3 -m unittest discover -s example-code/rewriting-labs -v
PYTHONDONTWRITEBYTECODE=1 python3 -m unittest discover -s example-code/functional-labs -v
PYTHONDONTWRITEBYTECODE=1 python3 -m unittest discover -s support -p 'test_*.py'
```

The three lab suites supply **14 test methods**. Detailed counts:

- 7,905 exact text/pattern comparisons against direct matching.
- 484 complete FST candidate-set comparisons against independent row DP,
  encompassing 14,520 query/radius/dictionary-word obligations.
- Canonical equivalence, case-fold expansion/frequency collision, ranking,
  empty/invalid cases, exhaustion, and spellchecker CLI status/output.
- 49 small unification pairs (every returned substitution makes both terms
  equal), direct/indirect occurs rejection, repeated-variable consistency,
  50 rewrite wrapper depths under two strategies, exact step counts and replay.
- Critical overlap and repaired-peak weight checks, string traces, shared graph
  update, cycle rejection, dangling references, malformed certificate rejection,
  and explicit target-cover costs.
- 507 generated higher-order functional programs across 13×13 operand pairs and
  three primitives, checked through five source/compiled/optimized/lazy paths
  against independent expected arithmetic; post-optimization type inference.
- List-sum/factorial, lexical capture/shadowing, polymorphic identity,
  environment-restricted generalization, malformed/ill-typed input, thunk
  sharing/black holes, strict/lazy divergence distinction, and all CLI modes.

All **37 shared support tests** also pass. Logs are outside the repository:
`/tmp/opencode/string-tests.txt`, `rewrite-tests.txt`, `functional-tests.txt`, and
`strings-rewriting-support.txt`.

## Manuscript verification

The full XeLaTeX/Biber/makeindex build is run with the repository's established
temporary Fontconfig and Biber runtime environment:

```sh
FONTCONFIG_FILE=/tmp/opencode/21cc-build/fonts.conf \
LD_LIBRARY_PATH=/tmp/opencode/21cc-build/usr/lib texlua build.lua
python3 support/export-artifacts.py --check --strict
texlua build.lua check
git diff --check
```

Strict serialization verifies **155 algorithms, 178 TikZ diagrams, and 169
Markdown tables** for the complete manuscript. New cross-references/citations
resolve; labels are globally unique; all new algorithms/figures/tables are
captioned, labeled, and referenced. All expected chapter sources are present.

The final full build succeeds and produces a **684-page** `index.pdf`, compared
with the preceding 580-page manuscript. Its final log has no undefined
references/citations, duplicate labels, or Biber warnings, and no new overfull
boxes. The pre-existing approximately 0.68-point overflow remains outside this
expansion. All **51 test methods** (14 new-lab and 37 shared-support) pass.

All 17 chapters and 34 diagrams were visually reviewed using rendered pages under
`/tmp/opencode/21cc-build/strings-rewriting-review/`. Review corrected an overwide
path, table columns, and an overlong list-of-figures caption. Several isolated
further-reading endings were reflowed through substantive proof/example additions
or concise wording, without manual page breaks or chapter-local spacing hacks.
The repaired-peak termination example now uses a checked weighted interpretation;
ordinary node count would not strictly decrease the constant-to-constant rule.

Build transcript: `/tmp/opencode/strings-rewriting-full-build.txt`.
Text extraction: `/tmp/opencode/strings-rewriting-final.txt`.
Saved final PDF and logs: `/tmp/opencode/21cc-build/strings-rewriting.pdf`,
`strings-rewriting.log`, and `strings-rewriting.blg`.
The root `index.pdf` remains available; ordinary auxiliaries are cleaned after
archiving. LuaLaTeX and exhaustive reinspection of older chapters are not claimed.

No new TODO markers or dependencies/packages were introduced. No commits or
pushes were made, and earlier whole-book verification gaps remain separate.
