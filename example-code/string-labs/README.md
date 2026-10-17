# String algorithms and FST spellchecking

Original companions for stable source chapters 77–80. Python 3.11+ and its
standard library are the only dependencies. No dictionary corpus is downloaded.

```sh
PYTHONDONTWRITEBYTECODE=1 python3 -m unittest discover -s example-code/string-labs -v
python3 example-code/string-labs/strings.py example-code/string-labs/dictionary.tsv ct --radius 1
```

The second command returns cat, cot, and cut, all at cost one, in frequency order.
The TSV dictionary is a tiny original example, not a linguistic benchmark.

## Implemented algorithms

- Prefix-function construction and all-occurrence exact matching, including
  overlapping matches. An empty pattern matches all sequence boundaries.
- NFC → casefold → NFC canonical keys using the host's Unicode database.
  Python strings supply code-point indexing; strict UTF-8 decoding is used for
  dictionary files. Record `unicodedata.unidata_version` for persisted indexes.
- A trie identity finite-state transducer composed implicitly with unit-cost
  insertion, deletion, and substitution arcs. Product states are query position
  and trie node. Copy costs zero; all three edit operations cost one. An input
  deletion consumes query only; an insertion emits dictionary text only.
- Nonnegative shortest-path search with stale-entry filtering, best cost per
  product state, complete within-radius candidate enumeration, and ranking by
  `(distance, -frequency, canonical_word)` before truncation to the result limit.
- A separate row-DP Levenshtein implementation used only as an independent oracle.

This is an actual specialized FST composition, not a generic arbitrary-transducer
library and not a string-edit enumerator disguised as one. The trie emits its
own spelling and recognizes final words. Its unique-prefix invariant permits
one best distance per state pair without losing distinct output spellings.
Do not substitute a minimized suffix-sharing DAG without revisiting that proof.

## Contracts and limits

Dictionary words are nonempty strings, and frequencies are positive exact
integers. Queries can be empty. Word/query extents are limited to 128 code points
both before and after normalization. Radius is 0–3. Result limit and expansion
budget are positive integers. Normalized dictionary collisions sum frequencies;
outputs use canonical spelling rather than original display variants.

Distance is measured in normalized Unicode **code points**, not bytes or
grapheme clusters. Transposition is not a primitive edit. No tokenization,
morphology, context language model, keyboard model, or locale-specific collation
is implemented. Frequency is a secondary ranking key, not a probability model.

Budget counts nonstale expanded product states, not transitions, wall time, or
total bytes. Exhaustion raises `BudgetExceeded`; the CLI exits 2 and writes no
successful JSON. A completed empty candidate set returns `[]` successfully.
Construction and file reading have no separate hard dictionary-size quota, so
this is not a hardened untrusted-input service. Trie prefixes are stored in full
for clarity, adding a maximum-word-length factor to storage.

## Verification

`test_strings.py` runs four test methods:

- all 7,905 text/pattern pairs from binary texts up to length seven and patterns
  up to length four, checked against direct start-position matching;
- 484 complete ranked candidate-set comparisons: 121 queries over `abc` through
  length four, 30 nonempty dictionary words over `ab` through length four, and
  four radii; 14,520 independent word-distance obligations;
- canonical equivalence, case-fold expansion, merged frequencies, example
  ranking, and explicit unsupported/invalid bounds;
- subprocess CLI success and budget-failure status/output checks.

The chapters also explain UTF-8 validation, ropes, suffix indexes, general
weighted composition, semirings, and determinization limitations. These are
design/exposition topics, not additional implementations hidden in this lab.
Exercise answers are in `solutions.md`.
