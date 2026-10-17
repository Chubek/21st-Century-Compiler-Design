# Distributed compiler-architecture expansion

## Approved scope and reading order

Eight **new** chapters are interleaved throughout the book. Existing source IDs
remain stable; `index.ltx` supplies printed numbering and `draft 61` still selects
source `chapters/ch61.tex`. The inventory in `build.lua` now covers 68 sources.

| Source | Title | Follows |
| --- | --- | --- |
| 61 | Compilers as Incremental Databases | Static Single Assignment |
| 62 | Declarative Optimization and Equality Saturation | Value Numbering and Redundancy |
| 63 | Semantic, Memory, and Effect-Aware IRs | Memory Optimization |
| 64 | Binary Lifting and Cross-Architecture Translation | Linker case study |
| 65 | Adaptive Interpreters and Staged Compilation | Virtual Machines and Bytecode |
| 66 | Speculative IR, Deoptimization, and State Transfer | Speculation and Deoptimization |
| 67 | Continuous and Runtime-Managed Compilation | New state-transfer chapter |
| 68 | Profile-Guided, Learned, and Spatial Compilation | Vectorization and Parallel Execution |

Supporting sections appear in sources 06, 12, 16, 21, 31, 34, 35, 36, and 60.
They explain the concrete connection (dependency kinds, retained alternatives,
memory clobbers, handler contracts, version ownership, recovery, device costs,
or lifting boundaries), with cross-references to the detailed chapter.

## Subject coverage

The numbering below is an editorial coverage checklist, not an attribution to
the unidentified paper in the original excerpt. The maintainer approved using
identifiable sources instead of waiting for that attribution.

| # | Subject | Main source and treatment |
| --- | --- | --- |
| 1 | Continuous/evolutionary compilation | 67: overlapping compilation, execution, observation, replacement |
| 2 | Representation tiers and representation “lattice” | 67: typed artifact graph; explicit qualification of lattice requirements |
| 3 | Speculative IR and deoptimization | 66: assumptions, guards, continuation relations, recovery recipes |
| 4 | Binary lifting and cross-ISA translation | 64: real byte instructions, flags, unknown control flow, environment boundary |
| 5 | Bidirectionality and provenance | 61: many-to-one origins and explicit update-policy limits |
| 6 | Graph/relational IR organization | 61: ordered operands, stable identities, snapshot integrity |
| 7 | Compiler databases | 61: facts, pure tracked queries, caching, transactional publication sketch |
| 8 | Demand-driven compilation | 61/65: query roots, lazy code entries, materialization |
| 9 | Declarative/query transformations | 61/62: candidate queries, semantic conditions, checked mutation |
| 10 | Rewriting and equality saturation | 62: actual e-graph, rebuilding, budgets, finite extraction |
| 11 | Learned costs | 68: fitted synthetic model, production source examples, generalization limits |
| 12 | Adaptive/self-optimizing interpreters | 65: quickening state machine, guards, superinstructions |
| 13 | Metacircular systems | 65: language relationship distinguished from self-application |
| 14 | Generated interpreters | 65: specification-driven generation distinguished from projections |
| 15 | Staging and Futamura projections | 65: residual bytecode, first/second/third projection equations |
| 16 | Semantic IRs | 63: state transitions, outcomes, lowering obligations |
| 17 | Explicit memory IRs | 63: memory tokens versus MemorySSA analysis overlay |
| 18 | Effect-aware IRs | 63: read/write sets, observations, traps, divergence, monadic organization |
| 19 | Temporal relationships | 68: defined affinity model, distinct from dependence/happens-before |
| 20 | Probabilistic execution information | 68: empirical branch/profile information versus program semantics |
| 21 | Versioned compilation | 67: request epochs and stale completion |
| 22 | Artifact reclamation | 67: retired versus collectible, lease-completeness condition |
| 23 | Incremental/reactive compilation | 61: ordered dependency validation and unchanged-result cutoff |
| 24 | Actor/distributed coordination | 67: immutable worker requests, serialized ownership, message ordering |
| 25 | Heterogeneous resources | 68: transfer-aware CPU/device threshold and semantic compatibility |
| 26 | Spatial placement | 68: exact tiny weighted-distance layout and real constraints |
| 27 | Uncertainty-aware optimization | 68: interval policy, horizon, budget, model boundaries |
| 28 | Runtime-owned optimization | 67/68: policy ownership, compilation service budgets, adaptive placement |

## Source verification

- Rust compiler implementation guide: ordered red-green dependency validation
  and unchanged result cutoff, consulted directly.
- Firm: local `sources/firm-graph-based-ir-il.txt` cross-checked against the KIT
  publication record (Braun, Buchwald, Zwinkau; technical report 35, 2011).
- MIL: local `sources/mil-monadic-ir-il-funlang.txt` title information and Crossref
  checked for Jones, Bailey, Cooper, IFL 2018, DOI 10.1145/3310232.3310238.
- LLVM MemorySSA: official documentation consulted for analysis-overlay semantics.
- egg: authors/title and Crossref publication metadata cross-checked against
  arXiv 2004.03082; DOI 10.1145/3434304.
- BAP: Crossref metadata and the project's official README consulted. Retrieval
  of the CMU-hosted paper failed; no detailed implementation claim relies on it.
- Byte x86 instructions: ADD, XOR, and MOV entries read from Félix Cloutier's
  instruction-reference mirror. This is identified as a mirror, not an Intel
  publication. ADD arithmetic is also independently checked against hardware.
- Self-optimizing AST interpreters: Crossref author/title/venue metadata verified;
  GraalVM implementation documentation consulted for specialization context.
  A guessed author-hosted PDF URL returned 404 and was not used as evidence.
- Futamura: Crossref metadata verified for the 1999 published article,
  DOI 10.1023/A:1010095604496; no invented original-publication date is assigned.
- Deoptimization: arXiv 1711.03050 and its PDF first pages checked. The article
  identifies January 2018, volume 2, POPL, article 49; Crossref's online date is
  December 2017. Citation uses the published article year and DOI 10.1145/3158137.
- Halide learned autoscheduling: official publication page and Crossref checked,
  DOI 10.1145/3306346.3322967. No benchmark values are copied or invented.
- LLVM ORC and MLGO: current official project documentation sources consulted;
  `.md` source URLs are recorded because the guessed old `.rst` paths returned 404.

All prose, examples, diagrams, algorithms, and exercises are original. Reference
extracts remain read-only. No claims are attributed to the unidentified paper.

## Executable verification

`example-code/architecture-labs/README.md` specifies dependencies and exact model
boundaries. Sixteen test methods pass, including:

- 100 incremental-versus-fresh query combinations, ordered dependency changes,
  early cutoff, and cycle cleanup;
- 256-input equality checks, constant folding, congruence, bounded-search and
  ungrounded-cycle behavior;
- all 16 concrete address pairs for store commutation;
- 65,536 ADD operand pairs against actual x86 flags in each of GCC, Clang,
  GCC ASan/UBSan, and Clang ASan/UBSan configurations;
- all 65,536 input/immediate pairs for the small MOV and XOR contracts using a
  separate bit-by-bit oracle, plus malformed/truncated instruction rejection;
- 3,417 source/residual comparisons for staging;
- 7,465 baseline/OSR/deoptimization comparisons across 1,555 input sequences,
  comparing outcomes and complete event traces;
- cyclic object identity, stale publication, invocation lease lifetime, 243
  arrays through three migrations each, and worked arithmetic/placement answers.

All 32 exercises have worked answers in `example-code/architecture-labs/solutions.md`.
The semantic-lowering checker, transactional publisher, general CFG recovery,
and production concurrency mechanisms are pseudocode/design contracts, not
unimplemented features advertised as working tools. The real lifter's executable
profile is exactly three unprefixed AL instruction forms. Learned and spatial
examples use synthetic cost data; no measured speedup is asserted.

## Build and export verification

Commands used:

```sh
texlua build.lua check
PYTHONDONTWRITEBYTECODE=1 python3 -m unittest discover -s example-code/architecture-labs -v
PYTHONDONTWRITEBYTECODE=1 python3 -m unittest discover -s support -p 'test_*.py'
python3 support/export-artifacts.py --strict
python3 support/export-artifacts.py --check --strict
FONTCONFIG_FILE=/tmp/opencode/21cc-build/fonts.conf \
LD_LIBRARY_PATH=/tmp/opencode/21cc-build/usr/lib texlua build.lua
```

The expansion adds 16 algorithms, 16 TikZ diagrams, and 16 tables, all serialized
by the existing exporter. Current corpus totals: 105 algorithms, 128 graphs,
and 119 tables. Source inventory checking passes. The 16 architecture tests and
37 support tests pass; logs are in `/tmp/opencode/architecture-labs-tests.txt`
and `/tmp/opencode/architecture-support-tests.txt`.

Visual review covered contact sheets for all eight new chapters and all sixteen
new diagrams. It caught an artifact-graph edge crossing the CPU-variant node;
the edge was rerouted outside the node. Two long inline math lists were rewritten
as breakable prose to remove margin overflow, and a redundant closing sentence
was removed to avoid an otherwise nearly empty final page in the lifting chapter.
The final full XeLaTeX/Biber/makeindex build passed on October 16, 2026:
`index.pdf` contains **544 pages**. Final logs contain no undefined references or
citations, multiply defined labels, or Biber warnings. The only overfull box is
the pre-existing approximately 0.68 pt one; no new overfull boxes remain.
The corrected artifact graph, the lifting chapter's closing page, and the final
e-graph and temporal-model pages were rechecked in the final PDF.

Final PDF and logs are preserved as `architecture.pdf`, `architecture.log`,
`architecture.blg`, and `architecture-final.txt` under
`/tmp/opencode/21cc-build/`. Rendered review pages are in its
`architecture-review/` subdirectory; the page inventory is
`/tmp/opencode/architecture-visual-pages.txt`. Root `index.pdf` is retained;
ordinary build auxiliaries are cleaned. `git diff --check` and strict export
synchronization pass. No commits or pushes were made.

## Remaining boundaries

These chapters do not establish textbook-wide completion. Earlier citation TODOs,
exercise implementation gaps, and the broader review obligations remain in the
existing review files. No new TODO markers are introduced in this expansion.
The architecture models do not constitute one integrated production JIT.
LuaLaTeX and a complete page-by-page audit of the older manuscript are not part
of this verification run.

During integration, the existing source-06 worked example was noticed to name
`t4` as a branch condition immediately after computing its comparison into `t5`.
That pre-existing example defect is recorded here for the broader correctness
review; this expansion adds a separate architectural connection section there.
