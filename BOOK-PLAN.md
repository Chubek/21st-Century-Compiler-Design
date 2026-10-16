# 21st Century Compiler

## Agreed scope

The maintainer approved the 38-chapter outline below and subsequently removed
all chapter word quotas. Choose length according to the material needed to
explain and verify each chapter's subject.

All 38 chapters and three appendices now have drafts. They are not a finished,
independently verified textbook. The third refinement pass strengthens algorithm
contracts, complexity arguments, examples, and exposition throughout the book;
see `THIRD-REVIEW.md` for corrections and verification records.
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
