# Rewriting laboratories

Original executable companions for source chapters 81–88. Python 3.11+,
standard library only. Run from the repository root:

```sh
PYTHONDONTWRITEBYTECODE=1 python3 -m unittest discover -s example-code/rewriting-labs -v
```

## Executed mechanisms

`rewrite.py` contains finite immutable first-order terms, a distinct `Var` type,
consistent matching, a finite-term unifier with occurs check, child-index
positions, persistent occurrence replacement, bounded normalization, certificate
replay, renamed-apart critical-pair enumeration, a bounded string normalizer,
finite graph read-back, a pure shared add-zero redirection, a restricted
dangling-edge deletion check, and a tiny additive target-tree selector.

Inputs to term utilities are trusted finite well-formed terms with consistent
symbol arities. There is no general external rule parser or global signature
validator. Rule left sides must not be variables, and right-side variables must
occur on the left. Negative normalizer fuel is rejected. Normalization reports
whether its final term actually has a redex, including after the last permitted
step. A self-loop rule therefore exhausts fuel without claiming a normal form.

`match` treats subject variables as fixed subject structure. Its replacement
instantiation is simultaneous, so a rule variable sharing an ID with a subject
variable does not recursively rebind the subject. `apply` is separately the
acyclic, transitively resolving substitution operation used by unification.
Arbitrary client-supplied cyclic substitutions are outside its contract.

Critical-pair enumeration renames variables apart and considers all nonvariable
left-side positions. It drops equal pairs but can retain duplicate nontrivial
pairs. It does not perform completion or prove arbitrary joinability. The
chapters explain completion, reduction orderings, indexed rule engines, DPO
graph transformation, SSA repair, and parallel rewrite conflicts as broader
design/theory topics, not implemented production subsystems.

## Graph and target restrictions

Graph fixtures use valid integer node IDs and arity-correct node payloads.
`unfold` materializes the finite tree and rejects a cycle on the active path.
It can cost exponentially more than the shared graph; it is not an infinite
rational-tree reader. `redirect_add_zero` assumes a valid acyclic graph of pure
total operations and zero constants; it updates one shared node into an
indirection. Both incoming uses change. No GC, use-list maintenance, concurrent
readers, or general graph-matching engine is implemented.

`delete_vertices` is a restricted supplied-match operation that deletes no edges.
It rejects any incident edge before removing selected isolated vertices. This
isolates the dangling hazard; it is not a complete DPO implementation.

`select_tree` accepts finite add/mul trees with leaf operands. Costs are abstract:
leaves zero, add two, multiply three, fused add(mul(a,b),c) three. No target
floating-point fusion claim is made. The small recursive fixture lacks the
memoized table of a production dynamic-programming selector and does not claim
general DAG optimality, register allocation, or hardware cycle prediction.

## Checks

Five test methods check repeated-variable matching, 49 finite unification pairs
(every returned map makes its input terms equal), direct and indirect occurs
behavior, 50 wrapper depths under two strategies, exact step counts and replay,
fuel exhaustion, invalid rules, a concrete unjoinable critical pair, overlap-free
identities, malformed certificate indices/positions/substitutions, string traces,
shared updates, cycle rejection, dangling references,
and the two legal target covers of the worked multiply-add tree.

These are finite tests for the specified mechanisms, not a proof of all possible
rule collections. Unrestricted equivalence, termination, and completion remain
outside an automatic total decision procedure. All 32 exercises have answers in
`solutions.md`.
