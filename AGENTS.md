# AGENTS.md

Guidance for AI coding and writing agents working in this repository. Read this
file fully before making changes. When it conflicts with a general habit of
yours, this file wins. When it conflicts with an explicit instruction from the
human maintainer in the current session, the maintainer wins.

---

## 1. Project overview

This repository is the source of a book about compilers and interpreters, with
a focus on optimization. It covers the middle end (dataflow analysis, SSA, loop
transformations), the back end (instruction selection, scheduling, register
allocation), and runtime systems (interpreters, virtual machines, JIT
compilation, garbage collection).

The book is typeset with LaTeX. The master file is `index.ltx`. It is split into
38 chapters, one file per chapter, included from `chapters/`.

The target reader is a programmer or graduate student who knows data structures,
basic discrete math, and at least one systems language. They want to implement
optimizations, not only read about them. Write for that person.

### Repository layout

```
.
├── AGENTS.md                  this file
├── build.lua                  build script (run with texlua)
├── Makefile                   convenience targets; read it before assuming
├── index.ltx                  master document
├── references.bib             bibliography (biblatex + biber)
├── chapters/                  chapter sources (ch01.tex ... ch38.tex)
├── example-code/              code that appears in, or backs, the book
├── sources/                   plain-text extracts of reference books (READ-ONLY)
└── support/
    ├── 21st-century-cc-book.cls   document class
    └── compiler-pseudocode.sty    pseudocode, math macros, IR listings, CFG styles
```

Facts you should not assume without checking: the exact contents of `Makefile`,
the exact contents of `example-code/`, and whether `chapters/` is already
populated. Look first, then act.

---

## 2. Ground rules

1. **Do not modify `sources/`.** Those files are reference material. Never edit,
   reformat, rename, delete, or commit changes to them.
2. **Do not reproduce copyrighted text from `sources/`.** See section 8. This is
   the most important rule in the file.
3. **Make the smallest change that does the job.** Do not reformat files you were
   not asked to touch. Do not rename macros, labels, or files as a side effect.
4. **Build before you claim success.** A LaTeX edit that you did not compile is
   an unverified edit. Say so if you could not build.
5. **Do not invent facts.** If you are unsure about a technical claim, a
   citation, a historical attribution, or a complexity bound, mark it with a
   `\todo`-style comment (section 6.9) or ask. A wrong statement in a textbook is
   worse than a gap.
6. **Do not invent citations.** Every `\cite{key}` must resolve to an entry in
   `references.bib`. Never fabricate authors, titles, years, venues, or page
   numbers.
7. **Ask when scope is unclear.** If a request could mean rewriting a chapter or
   fixing a typo, ask which.

---

## 3. Build system

### Toolchain

- Engine: **XeLaTeX** (default) or **LuaLaTeX**. pdfLaTeX will not work because
  the class uses `fontspec`.
- Bibliography: **biber** (biblatex backend).
- Index: **makeindex**.
- Build driver: `build.lua`, run with `texlua` (ships with TeX Live).
- Fonts (must be installed on the system): Source Serif 4, Source Sans 3,
  JetBrains Mono.

### Commands

```
texlua build.lua                 full build (engine, biber, makeindex, 2x engine)
texlua build.lua draft 3 4 7     build only chapters 3, 4 and 7 (fast, 2 passes)
texlua build.lua quick           single engine pass, no bibliography or index
texlua build.lua check           verify that expected source files exist
texlua build.lua fonts           verify that required fonts are installed
texlua build.lua clean           remove auxiliary files
texlua build.lua distclean       clean and remove the PDF
ENGINE=lualatex texlua build.lua use LuaLaTeX instead of XeLaTeX
```

`Makefile` may wrap these. Read it before assuming what a target does.

### Which build to run

- Editing one chapter: `texlua build.lua draft N`. In draft builds,
  cross-references into chapters you did not include show as `??`. That is
  expected, not an error.
- Changing citations, the index, or cross-chapter references: run the full build.
- Changing the class, the style file, or `index.ltx`: run the full build.
- Before reporting work as finished: run the full build and check the log.

### Known gaps in the current setup

Check these first, because they will cause build failures that look mysterious:

1. **`support/` is not on the TeX search path by default.** The class and style
   file live in `support/`, but `index.ltx` loads them by name. Either build with
   `TEXINPUTS=./support//:` set, or have `build.lua` set it. If the build fails
   with "File `21st-century-cc-book.cls' not found", this is why. Fix it in
   `build.lua` rather than copying the files into the root, so there stays a
   single copy of each.
2. **`build.lua` expects `frontmatter/preface.tex`.** That directory does not
   exist in the tree. Create it (with a minimal preface) or update the `extra`
   list in `build.lua` and the `\include` in `index.ltx`. Do not silently delete
   the check.
3. **`index.ltx` may still contain the original inline preamble** rather than
   `\documentclass{21st-century-cc-book}`. If so, and the maintainer wants the
   class used, migrate the preamble into the class (do not duplicate it) and keep
   everything from `\begin{document}` unchanged.
4. **Chapter files may not exist yet.** `build.lua check` reports missing ones.
   Use `draft` builds with only the chapters that exist.

### Build hygiene

- Never commit build products: `*.aux`, `*.toc`, `*.log`, `*.bbl`, `*.bcf`,
  `*.blg`, `*.idx`, `*.ind`, `*.ilg`, `*.out`, `*.run.xml`, `*.lof`, `*.lot`,
  `*.synctex.gz`, and the PDF unless the maintainer says otherwise. Keep
  `.gitignore` consistent with this.
- After a failed build, read the **first** error in the `.log`, not the last.
  Later errors are usually fallout.
- Treat these as problems to fix, not noise: overfull `\hbox` warnings wider than
  a few points, undefined references, undefined citations, and "multiply defined
  labels".

---

## 4. Chapter files

### Naming and structure

- Files are `chapters/ch01.tex` through `chapters/ch38.tex`, zero-padded to two
  digits. `build.lua` and `index.ltx` depend on this scheme. Do not rename.
- A chapter file contains **content only**: no `\documentclass`, no preamble, no
  `\begin{document}`. It starts with `\chapter{Title}` and a `\label{ch:slug}`.
- `\include` cannot be nested. Never put `\include` inside a chapter file. Use
  `\input` if you must split a long chapter into pieces, and put the pieces in a
  subdirectory such as `chapters/ch12/`.
- Each chapter ends cleanly; `\include` issues a `\clearpage` for you.

### Recommended internal structure

Unless the maintainer has specified otherwise for a given chapter, use this
shape:

1. A short opening that states the problem the chapter solves and why it matters
   in practice. No "In this chapter we will" boilerplate.
2. Concepts and definitions, introduced by example before formalism.
3. Algorithms, in `algorithm` floats, with a worked example on a small program.
4. Implementation notes: data structures, complexity, pitfalls, how real
   compilers (LLVM, GCC, HotSpot, V8, and so on) differ from the textbook
   version.
5. Exercises.
6. Further reading, as `\cite` references with one sentence on why each is worth
   reading.

### Labels and cross-references

Use prefixed, lowercase, hyphenated labels:

| Kind | Prefix | Example |
|---|---|---|
| Chapter | `ch:` | `\label{ch:ssa-construction}` |
| Section | `sec:` | `\label{sec:dominance-frontiers}` |
| Figure | `fig:` | `\label{fig:cfg-diamond}` |
| Table | `tab:` | `\label{tab:regalloc-comparison}` |
| Algorithm | `alg:` | `\label{alg:iterative-dataflow}` |
| Listing | `lst:` | `\label{lst:constant-folder}` |
| Equation | `eq:` | `\label{eq:dataflow-meet}` |
| Exercise | `ex:` | `\label{ex:liveness-loop}` |

Use `\ref` with a non-breaking space and an explicit noun: `Algorithm~\ref{alg:x}`,
`Figure~\ref{fig:x}`, `Section~\ref{sec:x}`. Never hard-code a number.

Labels must be globally unique across all 38 chapters. Before adding a label,
search for it.

---

## 5. Class and style file

### Rules for editing `support/`

- Treat `support/21st-century-cc-book.cls` and `support/compiler-pseudocode.sty`
  as shared infrastructure. A change there affects all 38 chapters.
- Before changing either, search `chapters/` for usages of whatever you are
  changing, and run a **full** build afterwards.
- Do not rename or remove a public macro or environment unless you also migrate
  every use in the same change.
- Do not add a package without a reason. Prefer what is already loaded. If you
  add one, say why in your summary and confirm it is in a standard TeX Live
  installation.
- Keep both files engine-compatible as documented in their headers. The class
  is XeLaTeX/LuaLaTeX only. The style file works with all three engines; keep it
  that way.

### Class: what it provides

Options: `a4paper` (default), `letterpaper`, `10pt`/`11pt`/`12pt`, `oneside`,
`monoligatures`. Other options pass through to `book`.

Commands and environments: `\subtitle{...}`, `\code{...}`, `\term{...}`,
`ccnote` and `ccwarning` callout boxes (optional title argument).

- `\term{new term}` emphasizes the term **and** adds it to the index. Use it at
  the definition of a concept, not on every mention.
- `ccnote` is for side remarks, practical tips, and historical asides.
  `ccwarning` is for pitfalls, undefined behavior, and common implementation
  bugs. Use them sparingly; more than two or three per chapter dilutes them.

### Style file: what it provides

**Pseudocode.** Use `algorithm` as the float and `pseudocode` for the body:

```latex
\begin{algorithm}
  \caption{Iterative dataflow analysis}\label{alg:iterative-dataflow}
  \begin{pseudocode}
    \Require CFG $G$, transfer functions $\Tf{B}$
    \Ensure fixed point $\In{B}$, $\Out{B}$
    \State $W \gets$ all blocks
    \While{$W \neq \emptyset$}
      \State remove some $B$ from $W$
      \State $\In{B} \gets \bigsqcap_{P \in \Pred(B)} \Out{P}$
      \If{$\Tf{B}(\In{B}) \neq \Out{B}$}
        \State $\Out{B} \gets \Tf{B}(\In{B})$
        \State add $\Succ(B)$ to $W$
      \EndIf
    \EndWhile
  \end{pseudocode}
\end{algorithm}
```

`\Require` prints "Input:", `\Ensure` prints "Output:", `\ForAll` prints
"for each", `\Comment{...}` is right-aligned, `\Break` and `\Continue` exist.

**Math macros.** Use these instead of ad-hoc notation so the book stays
consistent:

- Dataflow: `\In{B}`, `\Out{B}`, `\Gen{B}`, `\Kill{B}`, `\LiveIn{B}`,
  `\LiveOut{B}`, `\Tf{B}`, `\meet`, `\join`.
- SSA: `\phifn{...}`, `\assignphi{x_3}{x_1,x_2}`.
- CFG and dominance: `\Pred`, `\Succ`, `\dom`, `\idom`, `\DF`, `\Def`, `\Use`,
  `\Entry`, `\Exit`.
- Sets: `\set{x \in S | x > 0}`.

If a macro name clashes with another package, rename it in the style file and
fix every use. Do not work around it locally in a chapter.

**IR listings.** `\begin{irlisting}...\end{irlisting}` highlights three-address
code and LLVM-style IR.

**CFG figures.** TikZ styles `basic block`, `cfg edge`, `back edge`, and
`block label`. Reuse them. A CFG drawn with ad-hoc styles will look different
from every other CFG in the book.

---

## 6. Writing and typesetting conventions

### 6.1 Voice and style

- Direct, concrete, technically precise. Prefer short sentences. Prefer active
  voice and the present tense.
- Explain the *why* before the *how*. Show the failure that motivates an
  optimization before presenting the optimization.
- Introduce each idea on a small, concrete example before generalizing.
- Define a term at first use, with `\term`, and stay consistent with that term
  afterwards. Do not alternate between "basic block", "straight-line block", and
  "block" without reason.
- Avoid filler such as "it is important to note that", "obviously", and
  "trivially". If it is obvious, the reader does not need to be told; if it is
  not, the word is a lie.
- Avoid marketing language ("powerful", "state-of-the-art", "revolutionary").
- Use "we" for the author and reader working together; use "you" only in
  exercises and practical tips.
- American English spelling unless the maintainer says otherwise.
- Use the serial comma.

### 6.2 Technical content

- State preconditions and assumptions for each algorithm: what the input IR must
  look like, whether it assumes SSA, whether the CFG must be reducible, and so
  on.
- State complexity with the variables defined (for example "$O(N \cdot E)$ where
  $N$ is the number of blocks and $E$ the number of edges"). Distinguish
  worst-case from typical behavior when it matters, and say which one you mean.
- Where textbook and production practice differ, say so. Readers will
  eventually open LLVM or HotSpot and wonder why it looks different.
- Prefer correctness over breadth. A chapter that is right about fewer things
  beats one that is wrong about many.
- Do not present a heuristic as an optimal algorithm, and do not present a
  NP-hard problem's solution without saying it is a heuristic.
- When giving a worked example, **actually work it**: compute the sets, apply the
  transformation, show the result. Re-verify the arithmetic before committing.
  Dataflow tables and register-allocation examples are easy to get subtly wrong.

### 6.3 Notation

- One notation per concept across the whole book. Before introducing a symbol,
  check whether the book already uses one for that concept (search `chapters/`
  and `support/compiler-pseudocode.sty`).
- Names of analyses and passes in running text: set in plain text, capitalized
  as a proper name only when conventional (for example "Sparse Conditional
  Constant Propagation" at definition, then "SCCP"). Introduce the abbreviation
  once.
- Math in LaTeX math mode always, including single variables ($x$, not x).
- Use `\code{...}` for identifiers, opcodes, and short inline code. Use
  `irlisting` or `lstlisting` for anything longer than a line.
- Do not use `$...$` for plain-text units or words. Use `\text{}` inside math
  when needed.

### 6.4 Typography

- Non-breaking space before references and citations: `Figure~\ref{...}`,
  `see~\cite{...}`.
- Use `\emph{}` for emphasis and `\term{}` for definitions. Do not use bold in
  running text for emphasis. Do not use all caps for emphasis.
- Quotes: use `\enquote{...}` (csquotes), not straight quotes or hand-typed
  curly quotes.
- Dashes: `--` for ranges (`pages 10--20`), `---` for breaks in a sentence.
  Hyphens only for compounds.
- Do not insert manual line breaks (`\\`) or vertical spacing (`\vspace`) in
  running text. If a layout problem needs one, fix the cause or ask.
- Do not use `\newpage` or `\clearpage` inside chapters unless the maintainer
  asks. Let floats flow.
- Keep one sentence per source line, or break lines at clause boundaries. This
  keeps diffs reviewable. Do not hard-wrap at a fixed column mid-sentence.

### 6.5 Figures

- Put figure files in a `figures/` directory (the class sets
  `\graphicspath{{figures/}}`). Create it if it does not exist.
- Prefer vector formats (PDF, or TikZ in the source). Use PNG only for
  screenshots, and then at sufficient resolution.
- CFGs, dominator trees, and lattice diagrams should be TikZ using the shared
  styles, so they match the fonts and line weights of the book.
- Every figure has a caption and a label, and is referenced from the text. A
  figure that is not referenced is not finished.
- Captions are complete sentences that say what to notice, not only what the
  figure is.
- Do not rely on color alone to carry meaning. The book may be printed in
  grayscale.

### 6.6 Code listings

- Use `lstlisting` for source code and `irlisting` for IR. Both are configured
  by the class.
- Real code should come from `example-code/` (section 7), not be retyped into
  the chapter. If a listing is a fragment of a larger file, include the file
  segment, and keep the listing and the file in sync.
- Keep listings short enough to read: aim for under 40 lines. For longer code,
  show the essential part and refer to the file.
- Every listing that is discussed gets a caption and a `lst:` label, and the
  text refers to it by number.
- Do not break lines inside code in ways that change its meaning. Check that
  listings do not overflow the margin.

### 6.7 Tables

- Use `booktabs` rules (`\toprule`, `\midrule`, `\bottomrule`). No vertical
  rules, no `\hline`.
- Tables need a caption above them (figures get captions below), and a label.

### 6.8 Exercises

- Place exercises at the end of each chapter, graded roughly easy to hard.
- Each exercise is self-contained and has a `\label{ex:...}`.
- Prefer exercises that make the reader run the algorithm on a small program,
  implement a pass, or find the bug in a faulty transformation.
- Do not write exercises whose answers you have not worked out. If solutions are
  kept in the repository, they must match.

### 6.9 TODO markers

Use a consistent marker so the maintainer can grep for open items:

```latex
% TODO(agent): verify the complexity bound for the worklist variant
% TODO(cite): need a primary reference for loop-invariant code motion
% TODO(figure): CFG with a critical edge, before and after splitting
```

Use comments, not visible text, so TODOs never leak into the PDF. Leave a TODO
instead of guessing. Report outstanding TODOs you added in your summary.

### 6.10 Index

- Use `\term{}` at the definition of an important concept.
- For additional entries use `\index{...}`, with subentries
  `\index{register allocation!graph coloring}` and cross-references
  `\index{SSA|see{static single assignment}}`.
- One canonical entry name per concept, lowercase except proper names. Search
  existing `\index{` entries before adding one so you do not create
  near-duplicates.

---

## 7. Example code (`example-code/`)

Inspect the directory before relying on any assumption about it. General rules:

- Code that appears in the book must be **correct, compiles or runs, and is
  tested**. Run it. Do not paste code into a chapter that you have not executed.
- Keep each example self-contained where possible, with a short comment at the
  top saying what it demonstrates and which chapter uses it.
- Keep dependencies minimal and documented. If an example needs a toolchain
  (a particular compiler, Python version, and so on), record it in a README in
  that directory and in the file header.
- Match the language and style already used in the directory. Do not introduce a
  new language without asking.
- Keep examples small and readable, favoring clarity over cleverness or
  performance, unless the point of the example is performance.
- If you change an example that a chapter quotes, update the chapter in the same
  change and rebuild.
- Do not commit compiled binaries, build directories, or editor files.

---

## 8. Using `sources/` and handling copyright

`sources/` holds plain-text extracts of reference books, used as background
material. The names indicate works such as Appel's *Modern Compiler
Implementation in ML*, Muchnick's *Advanced Compiler Design and Implementation*,
Morgan's *Building an Optimizing Compiler*, Møller and Schwartzbach's *Static
Program Analysis*, Nielson, Nielson and Hankin's *Principles of Program
Analysis*, *The Garbage Collection Handbook*, Smith and Nair's *Virtual
Machines*, and others. Several files appear to cover overlapping static-analysis
material, and some may be duplicates or different editions. Do not assume which
is canonical. If it matters, ask.

These are copyrighted works. The book in this repository is an original work.
Follow these rules without exception:

1. **Never copy text from `sources/` into the book.** Not sentences, not
   paragraphs, not "lightly edited" versions of them. If your output closely
   follows the wording, sentence structure, or ordering of a source passage, it
   is copying even without quotation marks.
2. **Write from understanding.** Read the source to learn the idea, then close it
   and explain the idea in your own words, organized for this book's reader.
3. **Do not reproduce the source's examples, figures, tables, exercises, or
   pseudocode.** Create new examples. A worked example that matches a source
   example number for number is a copy. Algorithms themselves (as ideas) are
   free to describe; a source's particular presentation of them is not.
4. **Short quotation is the rare exception.** If exact wording matters (a
   definition whose precise phrasing is the point), quote at most a short phrase,
   in quotation marks, with a full citation, and only when the maintainer has
   agreed. Default to paraphrase.
5. **Cite the source.** When an idea, result, or technique is attributed to a
   work, cite it with `\cite{key}`. Prefer the original paper for original
   results (for example, the paper that introduced an algorithm) and a textbook
   for exposition.
6. **Do not copy bibliographies wholesale.** Add only entries you actually cite,
   and verify each one.
7. **Do not move, copy, or excerpt `sources/` files into other directories**, into
   commit messages, or into issue text.
8. **Be efficient and honest when searching them.** These files are large. Use
   targeted search (`grep -n`, `rg`) rather than loading a whole file into
   context. Text extracted from PDFs often has broken ligatures, hyphenation,
   lost math, scrambled tables, and missing figures; verify anything that looks
   odd, especially formulas and algorithm listings. Page numbers in extracts are
   unreliable, so cite by chapter and section, not by page, unless you can
   confirm the page against the real book.
9. **Cross-check technical claims across sources**, and against primary
   literature where possible. Books disagree, contain errata, and become dated.
   Where they disagree, say so or flag a TODO.

If you are asked to do something that would require copying source text (for
example, "paste the section on X"), decline that part, explain why, and offer a
paraphrased and restructured alternative instead.

---

## 9. Bibliography (`references.bib`)

- Add an entry for every work you cite. Do not cite what is not in the file.
- Key format: `lastname-year-keyword`, lowercase, for example
  `cytron-1991-ssa`. For multiple authors use the first author's surname. Look at
  the existing keys and follow whatever convention is already established if it
  differs from this one.
- Use the right entry type: `@book`, `@inproceedings`, `@article`, `@thesis`,
  `@online`. Fill in the fields the type needs (author, title, year, publisher or
  venue, and DOI or URL when one exists).
- Titles keep their capitalization via braces where needed:
  `title = {{SSA} Construction in Practice}`.
- Prefer stable identifiers (DOI, ACM DL, official proceedings page) over
  random mirrors. Do not add URLs you have not verified.
- If you are not certain of a bibliographic detail, leave a `note` field or a
  `% TODO(cite)` comment. Never guess a year, a venue, or a page range.
- Keep entries sorted or grouped consistently with how the file already is.
- Run the full build and make sure there are no "undefined citation" or biber
  warnings after your change.

Canonical references to seed from, when you can verify the details: Cytron et
al. on SSA construction; Chaitin and Briggs et al. on graph-coloring register
allocation; Poletto and Sarkar on linear scan; Kildall and Kam and Ullman on
dataflow frameworks; Cooper, Harvey and Kennedy on dominators; Lengauer and
Tarjan on dominator computation; Cousot and Cousot on abstract interpretation.
Treat this list as pointers for you to look up, not as ready-made entries.

---

## 10. Quality bar and verification

Before you consider a task done:

1. `texlua build.lua check` reports no missing files (or the missing ones are
   expected and you said so).
2. The full build succeeds, or a draft build succeeds for the chapters you
   touched and you say you did not run the full build.
3. No new undefined references, undefined citations, or duplicate labels.
4. No new overfull boxes beyond a few points.
5. Every new figure, table, listing, and algorithm is captioned, labeled, and
   referenced in the text.
6. Every new technical claim is either well known, cited, or marked with a TODO.
7. Every worked example has been recomputed.
8. Every code listing has been run.
9. No stray files: no build products, no scratch files, no changes under
   `sources/`.

When you finish, report briefly: what you changed, what you built and what you
did not, any TODOs you added, and anything you noticed but did not touch.

---

## 11. Git and change management

Only apply these if the repository is under version control. Check the existing
history for conventions, and follow them if they differ.

- Make small, focused commits: one chapter or one concern per commit.
- Commit messages: a short imperative summary line (about 50 characters), a
  blank line, then the reason for the change if it is not obvious.
  Examples: `Add dominance frontier algorithm to ch12`,
  `Fix liveness example in ch09`.
- Do not rewrite history, force-push, or amend commits you did not make unless
  asked.
- Do not commit generated files, secrets, or anything from `sources/` changed in
  any way.
- Do not make sweeping automated changes (global find-and-replace, mass
  reformatting) across the 38 chapters without confirmation. They are hard to
  review and cause merge conflicts.

---

## 12. Common tasks: how to do them

**Add a new section to a chapter.** Read the chapter first. Match its structure
and voice. Add a `sec:` label. Add `\term{}` for new concepts and `\index{}` for
extra entries. Cite sources. Run `texlua build.lua draft N`, then a full build.

**Draft a new chapter.** Confirm the outline with the maintainer if none exists.
Create `chapters/chNN.tex` using the structure in section 4. Draft in original
prose from your own understanding, using `sources/` only for learning (section
8). Include worked examples that you computed yourself, at least one algorithm in
the shared pseudocode style, and exercises. Leave TODOs rather than guesses.

**Add an algorithm.** Use `algorithm` plus `pseudocode`. Define inputs and
outputs with `\Require` and `\Ensure`. Use the shared math macros. Follow the
algorithm with its complexity, a worked example, and a note on how production
compilers implement it.

**Add a figure.** Use TikZ with the shared styles for graphs, or a vector file in
`figures/`. Caption, label, and reference it. Check it in grayscale.

**Add a citation.** Add the BibTeX entry to `references.bib` first, with verified
details, then `\cite{key}` in the text, then do a full build.

**Fix a build error.** Read the first error in `index.log`. Check the
"Known gaps" list in section 3. Fix the cause. Do not suppress the error with
`\batchmode`, `\nonstopmode` tricks, or by deleting content.

**Change the typography or layout.** Do it in `support/21st-century-cc-book.cls`,
not in individual chapters. Full build, then look at the output.

**Review a chapter.** Check, in order: technical correctness, whether examples
are worked correctly, notation consistency with the rest of the book, clarity,
and typesetting. Report findings as a list with chapter and section references,
and do not rewrite unless asked.

---

## 13. Things not to do

- Do not edit anything under `sources/`.
- Do not paste or closely paraphrase source text into the book.
- Do not fabricate citations, quotes, benchmark numbers, or historical claims.
- Do not add packages, fonts, or tools without saying why.
- Do not bypass or delete checks in `build.lua` to make a build "pass".
- Do not reformat, reorder, or rename files outside the scope of the task.
- Do not leave debugging output, scratch macros, or commented-out blocks of old
  text in committed files, other than the `TODO(...)` markers defined above.
- Do not commit build artifacts or the generated PDF unless told to.
- Do not assume the contents of `Makefile`, `example-code/`, or `chapters/`.
  Look.
- Do not claim a build passed if you did not run it.
