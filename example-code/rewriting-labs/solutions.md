# Rewriting exercise solutions

## 81 — Abstract and string rewriting

1. The two occurrences start at zero and one. They produce `ba` and `ab`, both
   normal. These are the only steps and hence the only reachable normal forms.
2. Number rules ab→a as zero and aa→a as one. Events are `(0,0)`, `(1,0)`,
   `(0,0)`, `(1,0)` in `(rule,offset)` order: ababa→aaba→aba→aa→a.
3. a→aa→aaa→aaaa. Fuel is exhausted and a redex remains. This does not alone
   prove divergence of arbitrary systems, though this particular rule has an
   independent endless-growth argument. The output is intermediate, not normal.
4. Natural-number length strictly decreases, excluding infinite steps. Overlaps
   can still disagree (aaa gives ba or ab), and a rule can consistently change
   the intended meaning. Neither confluence nor semantic correctness follows.

## 82 — Terms and unification

1. pair(x,x) against pair(a,a) binds x=a and verifies the other occurrence equals
   a. Against pair(a,b), the repeated check fails for distinct constants a,b;
   overwriting the old binding is incorrect.
2. Decompose into x=g(a) and g(y)=g(a), then y=a. Substitution on both originals
   yields f(g(a),g(a)). The map is most general up to irrelevant variable names.
3. Bind x=y. Dereference the later equation to y=f(y); y occurs inside its
   proposed value. Reject. Checking the original f(x) without following x's
   binding would miss the cycle.
4. Zero is at path `(0,1)`. Replacing only that occurrence gives
   mul(add(v,one),one), leaving the final outer one unchanged.

## 83 — Termination

1. Left size is `1+17+1=19`, right size 17, decrease two.
2. Difference is `10+x-(1+2x)=9-x`. Strict decrease holds for natural x from zero
   through eight, not for x=9 and not above it. A schematic instance at x=1
   therefore does not establish universal substitution stability.
3. The sole step returns v and a final redex search finds none: complete. Under
   f(x)→f(x), a redex remains after every allowed step, so status is incomplete.
4. add(a,b)→add(b,a)→add(a,b). Orient swaps only when operands violate a fixed
   canonical total order, under semantics permitting the reorder. A full
   associativity/commutativity normalizer also needs a canonical grouping policy.

## 84 — Confluence and completion

1. Overlap a→c at child path `(0)` of f(a)→b, with empty substitution. Common
   source f(a); descendants f(c) and b. With no other rules they do not join.
2. f(a)→b joins f(a)→f(c)→b. Ordinary node size does not decrease a→c, so a pure
   node-size argument is insufficient for the complete rule set. Use a weighted
   interpretation with a=2, c=1, b=0, f(x)=x+1: all three decrease and f is
   strictly monotone. This exposes why every rule, not just the repaired peak,
   must be checked.
3. Separate rule instances quantify their variables independently. Reusing IDs
   accidentally equates substitutions before unification and can remove real
   overlaps or manufacture constraints. Fresh left/right namespaces avoid it.
4. Only unresolved within that budget. A pair of actual forward reductions to
   the same term proves joinability. Complete descendant search can establish
   more in a known terminating finitely branching setting; bounded failure
   alone cannot.

## 85 — Strategies and engines

1. The index filters syntax, not semantic premises. Match both as appropriate
   and choose the first *admitted* rule whose complete conditions hold. Skipping
   the premise can apply an invalid transformation.
2. Update both reverse-use dependencies, invalidate every condition derived from
   the old operator/uses, and enqueue both parents (or a sound larger affected
   neighborhood). One traversal path does not identify all users of a shared node.
3. CFG edits can change dominance while leaving node IDs and types unchanged.
   Include an analysis epoch or invalidate/recompute the condition. Recheck at
   commit if discovery and publication can observe different IR versions.
4. Construct replacement privately, account its full retained size, validate
   invariants and budget, then publish atomically. On refusal, free only private
   uncommitted structures and retain the old valid IR and its interface.

## 86 — Term graphs

1. Height four uses five graph nodes and `2**5-1=31` unfolded tree nodes.
2. Before: pair(add(v,zero),add(v,zero)). After: pair(v,v). One node update
   changes two unfolded occurrences. An occurrence-only rewrite would require
   splitting sharing or path copying.
3. Pointer equality implies shared identity; pointer inequality permits two
   structurally equal representations. Unless canonical interning guarantees
   otherwise, it does not prove unequal finite unfoldings.
4. The constructor cycle exposes Cons before following its tail, so finite
   observations can succeed. A thunk that forces itself while busy has no value
   yet and triggers circular-demand detection. A finite-tree reader rejects
   either cycle because complete finite unfolding does not exist.

## 87 — Compiler graph transformation

1. With edge 0→1, deleting one leaves a dangling endpoint and is rejected before
   mutation. With no edges, output is vertices {0} and empty edges.
2. The right branch never defines w. A join use can execute along that path, so
   w does not dominate it. Supply a valid dominating computation or merge and
   prove semantic equivalence before replacing v.
3. Reclaim private new nodes and temporary validation state. The old visible
   graph, its uses, analysis versions, and published interface remain valid and
   unchanged. Do not free preserved nodes referenced by the failed replacement.
4. A rewrite can change the external use count seen by the other match even
   with no internal-node overlap. Both premises must be validated under a
   conflict/commit protocol; node-disjointness alone is insufficient.

## 88 — Optimization and selection

1. Separate cover: multiply three plus add two equals five. Fused root cover:
   madd three plus three zero-cost leaves equals three. These are abstract
   target units, not measured cycles or a floating-point equivalence proof.
2. Define loop(x)=loop(x) and evaluate mul(loop(0),0) strictly. Original diverges;
   replacement zero terminates. Pure does not mean total.
3. Two fused instructions may recompute the shared multiply, whereas a separate
   multiply can feed both adds. Registers, live ranges, scheduling, and operand
   constraints affect the real comparison. Additive tree costs omit these facts.
4. Establish the larger domain's exact overflow, exceptional, NaN, rounding,
   signed-zero, and effect behavior as applicable. A byte-domain test does not
   imply validity under a different arithmetic model. Prove or validate the rule
   under the actual target contract and retain its premises.
