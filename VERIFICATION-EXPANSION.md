# Logic, static analysis, and model checking

The maintainer approved **nine additional chapters**: two on logic, six on
static analysis/model checking and their compiler applications, and a case study
developing model-checking software. This is an expansion of the manuscript,
not a completion claim for all earlier chapters.

## Reading order and stable source IDs

| Source | Subject | Position in the book |
| --- | --- | --- |
| `chapters/ch40.tex` | Logic for Compiler Reasoning | After dominance, before dataflow |
| `chapters/ch41.tex` | Automated Reasoning and Its Limits | After logic |
| `chapters/ch42.tex` | Designing Sound Static Analyses | After abstract interpretation |
| `chapters/ch43.tex` | Relational and Path-Sensitive Analysis | After analysis design |
| `chapters/ch44.tex` | Interprocedural and Heap Analysis | After relational analysis |
| `chapters/ch45.tex` | Transition Systems and Temporal Properties | After interprocedural analysis |
| `chapters/ch46.tex` | Model-Checking Algorithms | After temporal properties |
| `chapters/ch47.tex` | Verifying Compiler Transformations | After model-checking algorithms |
| `chapters/ch48.tex` | Case Study: Developing a Model Checker | Before SSA and its optimization clients |

Printed chapter numbers follow `index.ltx`; draft arguments select the stable
source ID, e.g. `texlua build.lua draft 48`. Existing files and labels are retained.
The full inventory is 48 chapter sources and four appendices. All new diagrams,
algorithms, and tables have corresponding exports in the three artifact directories.

## Source and attribution checks

The new material uses original prose, examples, diagrams, algorithms, and
exercises. Reference extracts remain read-only. Filename identity was checked
against title/copyright pages rather than assumed.

- `logic-in-cs.txt`: Huth and Ryan, *Logic in Computer Science: Modelling and
  Reasoning about Systems*, second edition, 2004. Title/authors, edition, and
  paperback ISBN checked against the local front matter.
- `comp-and-logic.txt`: Boolos, Burgess, and Jeffrey, *Computability and Logic*,
  fifth edition, 2007. Title, authors, edition, publisher, and ISBN checked in
  the local title/copyright pages.
- `principles-of-prog-anal.txt`: Nielson, Nielson, and Hankin, *Principles of
  Program Analysis*, first edition 1999, consulted in the corrected 2005 printing.
- `model-checking.txt`: Baier and Katoen, *Principles of Model Checking*, MIT
  Press, 2008. Title, authors, ISBN, and catalog year checked locally.
- `handbook-of-model-checking.txt`: *Handbook of Model Checking*, edited by
  Clarke, Henzinger, Veith, and Bloem, Springer, 2018; DOI checked against the
  supplied front matter.
- Pnueli, Siegel, and Singerman, *Translation Validation*, TACAS 1998,
  pp. 151–166: author initials, title, venue, publication year, pages, and DOI
  verified using Crossref record `10.1007/BFb0054170`.
- Schmidt, *Data Flow Analysis Is Model Checking of Abstract Interpretations*,
  POPL 1998, pp. 38–48: author/title checked in the supplied nonempty extract,
  and venue/year/pages cross-checked through Crossref `10.1145/268946.268950`.

Some supplied extracts are incomplete: `dataflow-analysis-as-model-checking.txt`
contains only page separators. No claim is attributed to that empty extract.
The new dataflow/CTL connection is derived explicitly on an original small CFG
and supported by the analysis and model-checking references.

## Verification record

`example-code/translation-checks/checks.c` passes strict GCC and Clang C11
compilation and execution, both normally and with address/undefined-behavior
sanitizers. It checks two scalar rewrites on all 256 byte inputs, conditional
division on all 512 condition/divisor pairs, and paired unrolling states for
65 loop bounds. The chapter states the precise domain of these checks.
The liveness-as-until example was separately recomputed by fixed-point iteration.

The logic, analysis, and model-theory C11 checks also passed strict GCC and
Clang builds and address/undefined-behavior sanitizer runs. These recompute
truth tables, 64 Tseitin assignments, 510 width/input pairs, interval and DBM
results, recursive summaries, heap updates, temporal fixed points, and worked
exercise answers. The five new lab directories include 45 exercise solutions.

The model-checker test runner passed four configurations (GCC and Clang, strict
and sanitized). Each configuration checked 530 exhaustive small graphs,
28,868 safety queries, and 41,360 CTL queries against independent graph/path
oracles. It also checked malformed inputs, capacity limits, bit 63, exact
budgets, maximum-size input, shortest-witness replay, and CLI exit statuses.
The intentionally faulty cache model reports trace `0 1 2 3`; the corrected
model is safe. The implementation supports finite reachability and CTL
EX/EU/EG operators, with a 64-state capacity and explicit resource-limit status.
It is not a general LTL checker or a proof of a production runtime.

Commands used from the repository root:

```sh
sh example-code/model-checker/test.sh
PYTHONDONTWRITEBYTECODE=1 python3 -m unittest discover -s support -p test_export_artifacts.py
python3 support/export-artifacts.py --strict
python3 support/export-artifacts.py --check --strict
texlua build.lua check
FONTCONFIG_FILE=/tmp/opencode/21cc-build/fonts.conf \
LD_LIBRARY_PATH=/tmp/opencode/21cc-build/usr/lib texlua build.lua
```

The deterministic exporter has 22 passing tests. The synchronized inventory is
**59 algorithms, 76 TikZ graphs, and 75 Markdown tables**, including 15 algorithms,
18 diagrams, and 19 tables from this expansion. Original LaTeX and provenance are
retained. Strict export rejects unsupported conversions rather than silently
dropping content; the completed manuscript exports without warnings.

The full XeLaTeX/Biber/makeindex build passes at **397 pages**, with no undefined
references, undefined citations, duplicate labels, or Biber warnings. The existing
0.68 pt overflow remains; there are no new overfull boxes. Selected logic,
analysis, model-checking, validation, and case-study pages were visually checked.
LuaLaTeX and a page-by-page visual audit of the entire book were not run.
The draft driver was also corrected to forward every requested source ID;
previously its Lua expression passed only the first argument.
An unbraced dominator-loop argument was repaired for correct pseudocode binding
and unambiguous serialization.

No citation or technical TODOs were added by this expansion. Earlier manuscript
TODOs, including the type-theory/BBV/IR primary-reference gaps, remain separate
review work. Empty reference extracts were left intact. No commits were made.
