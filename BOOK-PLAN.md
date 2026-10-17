# 21st Century Compiler

## Agreed scope

### Strings, rewriting, and functional-language implementation

The maintainer approved 17 substantial new chapters, explicitly confirming an
**FST-based spellchecker**. Source IDs 77–93 preserve all earlier filenames.

| Source IDs | Chapters | Placement |
| --- | --- | --- |
| 77–80 | String representation/operations; matching/indexes; FSTs; spellchecker | After lexing overview |
| 81–88 | Abstract/string reduction; terms/unification; termination; confluence/completion; engines; term graphs; compiler graphs; optimization/selection | After SSA and incremental databases |
| 89–93 | Core semantics; inference/patterns; closures/lifting/combinators; lazy runtime; functional implementation | After type theory |

Each chapter includes explicit notation, worked examples, two algorithms, two
diagrams, two tables, and four solved exercises. The expansion supplies 34 of
each artifact and 68 exercise answers. The new artifacts use the strict exports.

`example-code/string-labs/` implements exact matching and a bounded Unicode
edit-FST/dictionary-FST spellchecker with an independent distance oracle.
`example-code/rewriting-labs/` implements finite rewriting and graph fixtures.
`example-code/functional-labs/` implements inference, optimization, a strict
interpreter/compiler/VM, and alternate lazy evaluation. The implementation/theory
boundaries and source checks are recorded in `STRINGS-REWRITING-FUNCTIONAL.md`.

### FFI, interoperability, and native extensions

The maintainer approved eight new chapters and the C11/libffi, Lua 5.4, and
CPython case studies. Source IDs remain stable and are interleaved as follows.

| Source ID | Chapter | Placement |
| --- | --- | --- |
| 69 | FFI Foundations and Semantic Contracts | After calling conventions |
| 70 | ABI Classification and Foreign-Call Lowering | After FFI foundations |
| 71 | Data Layout and Marshaling | After runtime representation |
| 72 | Ownership, Lifetimes, and Garbage Collection at Language Boundaries | After GC |
| 73 | Callbacks, Reentrancy, and Threads | After foreign ownership |
| 74 | Errors, Exceptions, and Unwinding across Language Boundaries | After callbacks |
| 75 | Native Extension Models | After dynamic linking |
| 76 | Case Study: Building an FFI and Native Extensions | After foreign errors |

`example-code/ffi-labs/` contains the native core, scalar libffi plans, Lua module
and embedded host, CPython Limited API module, ABI fixtures, finite models, and
32 solved exercises. All 16 algorithms, 16 diagrams, and 16 tables are serialized
through the strict exporter. `FFI-EXPANSION.md` records evidence and boundaries.

### Distributed compiler-architecture expansion

The maintainer approved eight new interleaved chapters, with supporting sections
in existing IR, SSA, redundancy, memory, bytecode, JIT, speculation, vectorization,
and linker chapters. Source IDs remain stable; printed numbers follow `index.ltx`.

| Source ID | Chapter | Placement |
| --- | --- | --- |
| 61 | Compilers as Incremental Databases | After SSA |
| 62 | Declarative Optimization and Equality Saturation | After value numbering |
| 63 | Semantic, Memory, and Effect-Aware IRs | After memory optimization |
| 64 | Binary Lifting and Cross-Architecture Translation | After the linker case study |
| 65 | Adaptive Interpreters and Staged Compilation | After virtual machines |
| 66 | Speculative IR, Deoptimization, and State Transfer | After speculation |
| 67 | Continuous and Runtime-Managed Compilation | After state transfer |
| 68 | Profile-Guided, Learned, and Spatial Compilation | After vectorization |

`example-code/architecture-labs/` contains executable reference models and solved
exercises. `ARCHITECTURE-EXPANSION.md` records scope, evidence, and model boundaries.
All new algorithms, graphs, and tables use the existing strict exporter.

### Assembly and linking expansion

The maintainer approved eight additional substantial chapters: four on assembly
and four on linking, with the two implementation case studies included.
The selected target is **x86-64 and ELF64**, with C11 implementations embedding
Lua 5.4 for assembler preprocessing and linker memory mapping.

| Source ID | Chapter |
| --- | --- |
| 53 | Assembly Languages, Expressions, Symbols, and Sections |
| 54 | Instruction Encoding, Fixups, and Relocatable Objects |
| 55 | Lua Preprocessing, Macros, and Multi-Pass Layout |
| 56 | Case Study: Building a Lua-Preprocessed Assembler |
| 57 | Symbol Resolution, Archives, and Separate Compilation |
| 58 | Memory Layout, Relocation, and Executable Images |
| 59 | Dynamic Linking, Loading, TLS, and Link-Time Transformations |
| 60 | Case Study: Building a Linker with Lua Memory Maps |

Assembly chapters follow out-of-SSA; linking chapters follow the existing linking
overview. `example-code/lua-toolchain/` contains the implementations, verification
driver, native examples, chapter computation checks, and forty worked exercise
answers. See `ASSEMBLY-LINKING-EXPANSION.md` for source and verification evidence.

### Lexing and parsing expansion

The maintainer approved four additional chapters, including the Aurocks case study,
and retaining C output from the supplied Perl generator:

| Source ID | Chapter |
| --- | --- |
| 49 | Regular Languages and Lexer Construction |
| 50 | Predictive Parsing and Grammar Design |
| 51 | Backtracking and Generalized Parsing |
| 52 | Case Study: Building Aurocks |

These follow the introductory parsing chapter. They include substantial pseudocode,
original worked diagrams, computed tables, checked exercises, and executable tests.
`support/aurocks.pl` generates C99 with cross-call backtracking, deferred actions,
explicit ownership, and bounded search. See `FRONT-END-EXPANSION.md`.

### Logic, analysis, and model-checking expansion

The maintainer approved eight additional chapters and a ninth chapter developing
a working model checker. Existing filenames and labels remain stable; printed
chapter numbers follow the include order in `index.ltx`.

| Source ID | Chapter | Placement |
| --- | --- | --- |
| 40 | Logic for Compiler Reasoning | After dominance, before dataflow |
| 41 | Automated Reasoning and Its Limits | After logic |
| 42 | Designing Sound Static Analyses | After abstract interpretation |
| 43 | Relational and Path-Sensitive Analysis | After analysis design |
| 44 | Interprocedural and Heap Analysis | After relational analysis |
| 45 | Transition Systems and Temporal Properties | After interprocedural analysis |
| 46 | Model-Checking Algorithms | After temporal properties |
| 47 | Verifying Compiler Transformations | After model-checking algorithms |
| 48 | Case Study: Developing a Model Checker | Before SSA and optimization clients |

These chapters develop compiler-specific contracts, original worked examples,
algorithms, diagrams, tables, and checked exercises. The case study supplies a
self-contained C11 implementation. Every new algorithm, diagram, and table is
also serialized in `pseudo-code/`, `graphs-code/`, or `tables-code/`.
The historical outline and installment records below predate this expansion.

The maintainer approved the 38-chapter outline below and subsequently removed
all chapter word quotas. Choose length according to the material needed to
explain and verify each chapter's subject.

All 93 chapters and four appendices now have drafts, including the type-theory
chapter (`ch39.tex`) and the compiler-foundations appendix (`appendix-d.tex`).
They are not a finished,
independently verified textbook. The third refinement pass strengthens algorithm
contracts, complexity arguments, examples, and exposition throughout the book;
see `THIRD-REVIEW.md` for corrections and verification records, and
`VERIFICATION-EXPANSION.md` for the nine-chapter expansion.
Outstanding citation TODOs and implementation exercises remain review work.
The chapter-status table and installment reports below are historical records
of the earlier drafting stage, not the current file inventory.

Write and verify it in installments. An installment is not a completed chapter.
Do not use placeholder chapter files to make the complete-book check pass.
Do not pad chapters with repeated explanations, copied source material, code,
bibliography entries, or generated text to satisfy a word target.

Count prose, headings, and captions with TeXcount. Exclude executable code,
pseudocode, mathematical symbols, bibliography entries, and index entries from
the length target. Retain the command and tool version with later word audits.
Technical accuracy, tested examples, and readable explanations remain separate
requirements; a word count does not establish any of them.

Executable examples use the recommended C11 baseline. Non-pseudocode listings
use `minted`; algorithms use the shared `pseudocode` environment. Every chapter
needs notation, worked computations, original graphs/diagrams, runnable examples,
exercises with checked answers, verified bibliography citations, and index terms.

## Chapter outline

| No. | Title | Status |
| --- | --- | --- |
| 1 | Programs, Meaning, and Translation | Three installments; incomplete |
| 2 | A Small Language and Its Interpreter | Not drafted |
| 3 | Lexical Analysis | Not drafted |
| 4 | Parsing | Not drafted |
| 5 | Names, Types, and Diagnostics | Not drafted |
| 6 | Intermediate Representations | Not drafted |
| 7 | Control-Flow Graphs | Not drafted |
| 8 | Dominance and Loops | Not drafted |
| 9 | Dataflow Analysis | Not drafted |
| 10 | Lattices and Fixed Points | Not drafted |
| 11 | Abstract Interpretation | Not drafted |
| 12 | Static Single Assignment | Not drafted |
| 13 | Local Simplification | Not drafted |
| 14 | Constant Propagation | Not drafted |
| 15 | Dead Code and Control Dependence | Not drafted |
| 16 | Value Numbering and Redundancy | Not drafted |
| 17 | Code Motion | Not drafted |
| 18 | Loop Analysis | Not drafted |
| 19 | Loop Transformations | Not drafted |
| 20 | Alias and Pointer Analysis | Not drafted |
| 21 | Memory Optimization | Not drafted |
| 22 | Interprocedural Analysis | Not drafted |
| 23 | Inlining and Specialization | Not drafted |
| 24 | Machines, Calling Conventions, and Object Files | Not drafted |
| 25 | Instruction Selection | Not drafted |
| 26 | Instruction Scheduling | Not drafted |
| 27 | Register Allocation | Not drafted |
| 28 | SSA Destruction and Machine Optimization | Not drafted |
| 29 | Linking and Loading | Not drafted |
| 30 | Interpreter Design | Not drafted |
| 31 | Virtual Machines and Bytecode | Not drafted |
| 32 | Runtime Representation | Not drafted |
| 33 | Garbage Collection | Not drafted |
| 34 | Just-in-Time Compilation | Not drafted |
| 35 | Speculation and Deoptimization | Not drafted |
| 36 | Vectorization and Parallel Execution | Not drafted |
| 37 | Testing and Validating Compilers | Not drafted |
| 38 | Building and Measuring an Optimizing Compiler | Not drafted |

The master file also reserves three appendices. Their subjects are not yet
agreed; no appendix text or placeholder has been created.

## Chapter 1 installment

The opening installment establishes a pure modulo-256 expression language,
an evaluator, a constant folder, its structural proof, storage contracts,
finite-domain tests, and limits on the claims these justify. It introduces
observable behavior, control flow, analysis, refinement, phase contracts, and
cost objectives through that example and a separate effectful C program.

The second installment adds a stack-code compiler, machine semantics, a worked
instruction trace, a stack-prefix preservation theorem, structural verification,
capacity-failure contracts, exact stack-demand calculations, and composition
tests before and after folding. The third adds an isolated wrong-code mutant,
a deterministic reducer, a complete five-step reduction trace, a minimality
argument, regression-test guidance, and checked exercises.
Current audited length is **13,179 words**,
including headings and captions and excluding code, pseudocode, and equations.
Command:

```sh
texcount -inc -opt=support/texcount.opts chapters/ch01.tex
```

The shared options exclude pseudocode, index entries, and minted file arguments;
the sum weights count only text, headings, and captions.
TeXcount is version 3.1.1, supplied by TeX Live 2024; its installed Perl script
emits two precedence warnings on this host. These are tool warnings, not LaTeX
errors. No chapter is presently within the required length range.

### Remaining chapter work

Continue Chapter 1 before declaring it complete. Develop substantive, checked
material rather than stretching the opening explanations:

- Develop the boundary between total pure expressions and operations with
  defined traps, effects, and nontermination, using new concrete examples.
- Audit host-versus-source arithmetic in depth, with defined C test cases and
  specification-based rules rather than executions of undefined behavior.
- Work through a pass contract, structural verification, invalidated facts,
  diagnostics, and a minimized failing test as one integrated engineering lab.
- Add a reproducible measurement lab that separates preparation cost, execution
  cost, and output checking. Report actual conditions and results, not invented
  performance claims.
- Expand exercises and implementation checks to cover the added material,
  then audit notation, citations, examples, figures, and word count together.

The current checkpoint, exercises, and further reading are provisional chapter
end matter. Move them after the new substantive sections as the draft grows.
Chapter 2 remains responsible for a fuller language and interpreter project;
Chapter 1's translator should stay small enough to justify completely.

## Source and citation notes

`sources/` is read-only. No source text, examples, exercises, or figures are
reproduced in the manuscript. Current background reading uses targeted passages
from `appel-ml.txt`, `morgan-opt-cc.txt`, and `trustworthy-opt-cc.txt`.

- Appel's author-maintained ML book page confirms the title, author, publisher,
  and 1998 date used for that edition. DOI landing metadata gives a different
  date; the manuscript cites the author-identified edition and its verified URL.
- Morgan's extract has informal front matter with an inconsistent date. The
  entry uses the verified 1998 Butterworth-Heinemann edition record, not that
  informal date.
- Safonov's bibliographic details are present in the extract's title and
  copyright pages.
- The C rules are checked against WG14 draft N1570, fetched from the committee
  site. It is identified as a committee draft, not the published ISO standard.
- `muchnick-book.txt` appears to contain a different work. Do not cite it as
  Muchnick without establishing its identity. It was not used for chapter claims.

Future chapters must identify and cross-check the relevant sources in the same
way. Do not populate the bibliography with books that the manuscript never cites.

## Verification of the opening installment (historical)

- XeLaTeX partial build: `texlua build.lua draftfull 1`, using the temporary
  dependency environment documented in `README.md`; 30 PDF pages including
  front matter, bibliography, and index. LuaLaTeX has not been tested.
- Final log: no undefined references/citations, duplicate labels, or package
  warnings. Three minor overfull boxes remain, all under 1.6 pt; no large
  overflows. Diagrams, code listings, bibliography, and index were inspected
  in rendered pages.
- Biber resolved all four cited entries. Makeindex generated the index without
  rejected entries or warnings. A source audit found 50 unique labels and
  resolved all 26 explicit references.
- GCC and Clang: all three example programs built under C11 with strict
  warnings treated as errors; tests and expected demo outputs passed. The
  test suite also passed both compilers' address/undefined-behavior sanitizers.
- TeXcount's exclusion rules were tested on a separate fixture; only its four
  prose/caption words counted. The chapter audit is 7,351 words as recorded above.
- Both the complete-book source check and complete-book build were attempted.
  They stop on the expected 40 unwritten chapter/appendix files; a complete-book
  build has **not** succeeded.
- One `TODO(agent)` remains in Chapter 1: finish its planned material and meet
  the agreed chapter-length range. No technical uncertainty was hidden with an
  invented claim or citation.

Generated files are removed from the manuscript tree after verification. A
copy of this installment's PDF and build transcripts is retained outside the
repository under `/tmp/opencode/21cc-build/` for this working environment.

## Verification of the second installment

- Chapter count: 10,507 words across `ch01.tex` and its input file, using the
  documented TeXcount command with `-inc`. This remains below 28,000 words.
- XeLaTeX/Biber/makeindex partial build passed: 38 pages. No undefined references,
  undefined citations, duplicate labels, or package warnings. The same three
  minor overfull boxes remain below 1.6 pt; a new exercise overflow was fixed.
- Source audit: 71 unique labels and 35 resolved explicit references in the
  chapter and its new input file. New trace, diagram, and listings were inspected.
- The new C11 stack tests passed with GCC and Clang, strict warnings as errors,
  and each compiler's address/undefined-behavior sanitizers. Arithmetic tests
  cover every literal operand pair for both operations; generated-tree tests
  compare source and target before and after folding for every input.
- New exercise answers were worked out and recorded in `SOLUTIONS.md`.
- Complete-book check and build were attempted again and stopped at the same
  40 unwritten files. LuaLaTeX was not tested. No new TODO was added; the existing
  chapter-completion TODO remains.
- `sources/` was not changed. Generated build files were removed from the
  manuscript tree; the updated PDF is `/tmp/opencode/21cc-build/ch01-draft.pdf`.

## Verification of the third installment

- TeXcount: 13,179 words including chapter pieces; still below the required
  minimum. No chapter has been declared complete.
- XeLaTeX/Biber/makeindex partial build passed: 46 pages. No undefined
  references/citations, duplicate labels, or package warnings. The same three
  minor overfull boxes remain below 1.6 pt. New code listings, reduction table,
  and diagnostic diagram were visually checked.
- Source audit: 86 unique chapter labels and 40 resolved explicit references.
- All five C programs passed GCC and Clang builds with strict warnings as
  errors. Expression, stack, and fault-lab tests also passed both compilers'
  address/undefined-behavior sanitizers. The exact six-line numeric reduction
  trace was checked automatically, and the new arithmetic exercise's count of
  128 agreeing operand pairs was checked exhaustively.
- Both complete-book commands were attempted; the same 40 undeveloped files
  prevent a complete-book build. LuaLaTeX was not tested.
- No new TODO was added. The existing chapter-completion TODO remains.
  `sources/` remains unchanged. Build products were removed from the manuscript
  tree; the current PDF is retained at the temporary path documented above.
