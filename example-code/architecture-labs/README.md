# Compiler architecture labs

Small executable models for source chapters 61–68. Python 3.10+ and its standard
library are sufficient for the models. The hardware-oracle test additionally
requires Linux x86-64, GCC, and Clang; it runs both compilers normally and with
ASan/UBSan. It uses the compilers' inline-assembly extension, with `-mno-red-zone`
for the `pushfq`/`popq` sequence. Temporary executables live under `/tmp/opencode`
(create that directory before running outside the development environment).

From the repository root:

```sh
PYTHONDONTWRITEBYTECODE=1 python3 -m unittest discover -s example-code/architecture-labs -v
```

## Contracts and boundaries

| Source | Executable model | Boundary |
| --- | --- | --- |
| 61 | `Queries`: revision stamps, ordered dependency validation, early cutoff | Single-threaded, pure providers, immutable inputs/results, acyclic queries; no persistent storage or transaction implementation |
| 62 | `EGraph`: union, rebuilding, identities, constant folding, extraction | Pure total byte arithmetic, one variable, valid arities and byte constants; no general matcher, effectful terms, or machine-code extractor |
| 63 | `Effect`, `commute`, concrete store execution | Sequential valid ordinary cells, sound explicit region sets, no value dependency; not an alias analyzer or concurrent memory model |
| 64 | `lift`, `execute_lifted`, `flags.c` | Exactly unprefixed `B0 ib`, `04 ib`, `34 ib`; AL and six flags; no CFG recovery, memory, prefixes, or host code emission |
| 65 | `AddSite`, expression interpreter, residual stack bytecode | Exact integers/strings for quickening; trusted finite integer expression trees for staging; no generic self-applicable partial evaluator |
| 66 | Baseline, guarded loop, OSR prefix, virtual-object materialization | Exact host integers, explicit trace oracle, chosen loop-header continuation, valid recipes; no native frames or collector integration |
| 67 | `Runtime`, `ArrayHandle` | Serialized owner and invocation leases, symbolic code, sequential migration; no threaded publication, escaped code pointers, or active-frame invalidation |
| 68 | Expected cost, least-squares fit, exhaustive unit-slot placement | Synthetic costs, nondegenerate samples, symmetric nonnegative weights; not measured speedups or a trained production model |

The EGraph API is a trusted construction API: `const` payloads must be integers
0–255, `var` is its one symbolic input, and `add`/`mul` take exactly two children.
Call `rebuild` after external unions before depending on canonical hash-consing.
`saturate` returns false on a resource stop; its node threshold is checked after a
rewrite step and can be exceeded by the bounded additions of that step. Extraction
still has the original finite term. It minimizes positive unit *tree* cost, not
DAG or register-allocation cost. Witness tuple construction and eventual tree
output have costs beyond scalar cost relaxation.

Query providers must not mutate objects returned by `get`, edit inputs while
evaluating, change provider definitions without invalidating the cache, or read
untracked global state. Cache equality is exact host equality. It must describe
the intended semantic equality (the tests use simple immutable integers/strings).
The direct deletion of an obsolete input in one test is a deliberate adversarial
fixture for dependency-order validation, not a public edit API.

The lifter expects `rax` in 0..2^64-1. Initial flag dictionaries describe the six
modeled flags. `None` after XOR denotes unspecified AF, not unchanged AF. The
hardware oracle validates ADD arithmetic through register-register `addb`; the
decoder's immediate form is checked separately. No arbitrary machine bytes are
executed by the Python lifter. Other host architectures skip the hardware test.

Runtime leases must come from the same owner and be released exactly once.
Epoch changes route new calls to a generic version while existing invocations
retain their old-version contract. This is a model of version ownership, not a
language policy for changing assumptions underneath an active optimized frame.
`ArrayHandle` migration retains identity and length; its reads require integer
indices and its payload is private. It is not a concurrent container.

## Evidence

- Incremental results compared with fresh query engines over 100 input pairs;
  unchanged-result cutoff, conditional dependencies, and cycle cleanup checked.
- E-graph simplification checked over all 256 input bytes, with constant-folding,
  congruence, resource-stop, and ungrounded-cycle checks.
- All 16 concrete address pairs checked for the store-reordering example.
- All 65,536 byte additions and six flags compared with x86 hardware in four
  compiler/sanitizer configurations; partial-register and rejection cases added.
- Staging compared with direct interpretation on 3,417 expression/input pairs.
- All 1,555 sequences of length 0–4 over six values tested at every OSR position
  (7,465 executions), including range failures and language errors.
- Cyclic object identity, stale publication, leased retirement, and all 243
  five-element arrays over three values checked through repeated migrations.
- Cost arithmetic, least-squares fitting, and exact tiny placement checked.

`solutions.md` gives worked answers for all 32 new exercises. Algorithm sketches
for transactions, general control-flow recovery, and semantic proof dispatch are
architectural contracts; the table above identifies which mechanisms are actually
implemented. Full-book compilation and exports are recorded in
`ARCHITECTURE-EXPANSION.md`.
