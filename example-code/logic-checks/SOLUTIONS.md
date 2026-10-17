# Logic chapter exercise solutions

Exercise identifiers refer to LaTeX labels, so these solutions remain valid
when chapter numbers change during integration. Arithmetic is mathematical
integer arithmetic unless a bit width is explicitly specified.

## Logic for Compiler Reasoning (`ch40.tex`)

1. **`ex:ch40-entailment`.** Set `p=false`, `q=true`, `r=false`.
   Both premises hold but the conclusion does not. Adding `not q` forces
   `p` through `p or q`; then `p implies r` forces `r`. Alternatively,
   the counterexample query with the additional premise has no valuation.

2. **`ex:ch40-capture`.** Rename the existential binder to fresh `z` first.
   The result is `exists z. y < z`. At free `y=5`, choose `z=6`, so the
   assertion is true. Naive replacement gives `exists y. y < y`, which
   is false for every integer. The binder must not capture the inserted
   free variable.

3. **`ex:ch40-sequence`.** Cross `b := 3*a` to obtain `3*a >= 15`;
   cross `a := a+2` to obtain `3*(a+2) >= 15`, equivalent to `a >= 3`.
   Initial `a=3` produces `b=15`; initial `a=2` produces `b=12`.

4. **`ex:ch40-branch`.** The final assignment requires `x >= 3`.
   Before the branch this becomes
   `(x < 0 implies -x >= 3) and (x >= 0 implies x+2 >= 3)`.
   The negative arm admits `x <= -3`; the nonnegative arm admits `x >= 1`.
   Thus the weakest precondition is `x <= -3 or x >= 1`, excluding
   exactly `-2`, `-1`, and `0`. Their final `y` values are `6`, `3`, and
   `6`, respectively. The boundary inputs `-3` and `1` both produce `9`.

5. **`ex:ch40-invariant`.** Use `0 <= i <= n and t = 5*i`, with unchanged
   `n >= 0`. Initialization satisfies it. A taken iteration gives
   `i' = i+1 <= n` and `t' = t+5 = 5*i'`. The variant `n-i` is a
   nonnegative integer that decreases by one; each body execution
   terminates. At exit `i=n` and `t=5*n`. For `n=3`, the successive
   `(i,t)` pairs are `(0,0)`, `(1,5)`, `(2,10)`, `(3,15)`.

6. **`ex:ch40-speculation`.** Take `b=false`, `d=0`. The premise is true
   and the original execution does not divide, but the moved division
   traps. Requiring `d != 0` on every incoming state is sufficient under
   the stated purity and totality assumptions. A fact known only on the
   true arm does not establish that premise before the branch.

## Automated Reasoning and Its Limits (`ch41.tex`)

1. **`ex:ch41-root`.** The definition clauses are
   `(not z or p)`, `(not z or not p)`, and `(z or not p or p)`.
   The final clause is tautological but may be retained. With unit `z`,
   the first two clauses force both `p` and `not p`, giving a conflict.
   Without the root assertion, `z=false, p=false` is a model; so is
   `z=false, p=true`.

2. **`ex:ch41-projection`.** There are four models with the root asserted
   and eight without it. Each valuation of the original three atoms
   uniquely determines the three auxiliary values. The gate definitions
   forbid all other auxiliary assignments, so there is no factor of eight.

3. **`ex:ch41-overflow`.** The exact condition is `0 <= x <= 12`, where
   the lower bound is already guaranteed by unsigned interpretation.
   In this range the sum is `x+3`, strictly larger than `x`. At `13`,
   `14`, and `15`, the wrapped results are `0`, `1`, and `2` respectively;
   these are all the counterexamples.

4. **`ex:ch41-width`.** Write `M=2^w` and `H=M/2`. For `0 <= x < H`,
   `2*x < M`, so the source evaluates to `x`. For `H <= x < M`,
   the modular sum is `2*x-M`, and unsigned division by two produces
   `x-H`. This differs from `x` because `H >= 1`. The exact validity
   condition is therefore `x < 2^(w-1)`; the erroneous result otherwise
   is `x - 2^(w-1)`. This argument covers every width `w >= 1`; the C
   checker independently enumerates widths 1 through 8.

5. **`ex:ch41-vc`.** The condition is `x0 <= -3 or x0 >= 1`, excluding
   exactly `-2`, `-1`, and `0`. On the negative arm, `y1=-3*x0`, so
   the goal requires `x0 <= -3`. On the nonnegative arm,
   `y1=3*x0+6`, so the goal requires integer `x0 >= 1`. Every excluded
   input has the violating execution computed in the branch exercise.

6. **`ex:ch41-unknown`.** The acceptance rule mistakes absence of a SAT
   answer for evidence of UNSAT. Require an UNSAT result for the
   counterexample query, together with the justified encoding and
   premises. UNKNOWN, timeout, cancellation, or solver failure cannot
   justify the rewrite. An UNSAT result for unrolling bound 20 rules
   out only the encoded bad executions within that bound. It does not
   establish absence of failure at iteration 21, unbounded correctness,
   or termination without additional reasoning.
