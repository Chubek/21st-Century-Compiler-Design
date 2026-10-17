# String chapter exercise solutions

Source IDs select files; printed numbering follows `index.ltx`.

## 77 — Representation and operations

1. UTF-8 bytes are `41 65 CC 81`, with scalar boundaries `[0,1,2,4]`.
   Scalar slice `[1,3)` maps to byte slice `[1,4)`, containing the e and accent.
2. Copying the existing prefix before each append copies
   `0+1+...+(n-1)=n(n-1)/2` bytes, plus n newly appended bytes if counted.
   Geometric capacity growth copies O(n) total bytes over n appends, assuming
   bounded per-byte copying and geometric allocation policy; an individual
   resize is still linear in the old content size.
3. Both keys are `strasse`. Their source occurrences retain distinct spelling,
   scalar/byte extents, and source positions. The canonical key is not a source
   replacement or a coordinate map back to either occurrence.
4. Equal lengths do not preserve contents, token boundaries, or declaration
   identity. The old span belongs to its source snapshot. Retain that snapshot
   or translate/revalidate the span through the edit history.

## 78 — Matching and indexes

1. Prefix function `[0,1,2,3]`; matches in six a's begin at 0, 1, 2.
   Falling back to border length three after a full match preserves overlaps.
2. After consuming `abab`, retain matched-prefix length four and the pattern's
   prefix table, plus global offset four. The next a completes the match at
   zero; continuing reports starts two and four. Resetting the prefix length
   at the split loses the crossing matches.
3. Root plus eight character-prefix states gives nine states. Prefix `compile`
   (depth seven) and prefix `compiler` (depth eight) are final. The first final
   has an outgoing r edge; final need not mean leaf.
4. Distinct prefixes can reach the same merged suffix state. Membership needs
   only acceptance, but enumeration must retain which prefixes were emitted.
   Keeping one prefix or predecessor per merged state can lose a valid word.

## 79 — Transducers and approximate matching

1. `ct → cat`: `c:c`, `epsilon:a`, `t:t`, cost one. `ct → coat`:
   `c:c`, `epsilon:o`, `epsilon:a`, `t:t`, cost two. The alphabet is normalized
   code points, and edit direction is query to candidate.
2. Distance is two: substitute both symbols, or delete a and insert it after b.
   There is no one-step Levenshtein insertion/deletion/substitution producing
   the swapped string. A model with a primitive adjacent transposition assigns
   one and is a different edit relation.
3. `(4+1)*31 = 155` possible state pairs before pruning. This is a state bound,
   not a bound on candidate count, transition-generation cost, or trie storage.
4. Tropical combination is `min(2,3)=2`; consecutive costs add along one path.
   Adding alternative costs would not compute minimum edit distance. Probability
   path sums require a separately defined weight interpretation and algebra.

## 80 — Spellchecker

1. At radius two, rank cat(1,30), cot(1,10), cut(1,5), coat(2,20).
   Limit three returns cat, cot, cut. Frequency never outranks lower distance.
2. One canonical entry `strasse`, frequency seven, distance zero. Both original
   spellings collapse under the explicit policy; display variants are not kept.
3. Heap order settles distance but does not rank every equal-cost word by
   frequency. A later final state can have equal distance and larger frequency.
   Complete the tied search or maintain a proven bound on the full ranking key.
4. The dictionary contains a zero-distance answer, but the search has not
   established its absence after only one expansion. Returning `[]` would claim
   a complete search with no result. Raise the explicit incomplete-search error;
   the CLI publishes no success JSON and exits two.
