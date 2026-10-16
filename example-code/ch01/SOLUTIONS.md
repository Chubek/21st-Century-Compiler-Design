# Chapter 1 exercise checks

These notes record the worked answers used to check the exercises. They are not
part of the typeset chapter. Arithmetic claims are also checked in `test_expr.c`.

1. For inputs 9, 249, and 255, the left additions yield 16, 0, and 6;
   the right addition yields 8 in every case. Final values are 128, 0,
   and 48. Every operation returns a value in 0..255 by definition.
2. The original tree has seven nodes. Postorder replaces `14+19` by 33,
   `6+5` by 11, and then `33*11` by 107 because 363 mod 256 is 107.
   One node remains reachable from the root. Disconnected storage remains
   caller-owned.
3. Evaluation of `e` terminates, produces a value in V, cannot trap, and
   has no effects. Its product with zero is zero modulo 256, and omitting
   its evaluation changes no other behavior. The C call changes a counter
   that contributes to output, so its omission changes an observation.
4. After folding children, shallow-copy the left child's struct into the
   parent (`*e = *e->left`) when the right child is a zero literal. This is
   safe under the stated caller-owned, non-sharing, acyclic contract: the
   child cannot be an ancestor of the parent, its storage remains alive,
   and struct assignment reads its value before storing the replacement.
   Do not free the child's storage or return the address of a local copy.
   For every x in V, `(x+0) mod 256 = x`; the same fact applies after
   computing `(7*x) mod 256`. The expression still evaluates its left
   operand and only omits a pure zero literal and the addition.
5. Assuming correct evaluation and valid trees, the particular original
   and result trees agree for every allowed runtime input. This does not
   prove agreement for other trees, bound stack usage, or establish that
   any simplification occurred. It also does not establish evaluator
   correctness.
6. Let `g[0]` be an input node; let `g[k]` be addition with both children
   pointing to `g[k-1]`. There are k+1 distinct allocated nodes and no
   cycles, but recursive visits satisfy T(k)=1+2*T(k-1), T(0)=1, hence
   T(k)=2^(k+1)-1. Input nodes prevent constant folding from collapsing
   the graph. A visited-node memo for folding avoids processing a node
   twice; a per-input value memo does the analogous job for evaluation.
   With appropriate memo access, a DAG traversal processes each node and
   edge a constant number of times. This is a different graph-aware
   algorithm with its own ownership and mutation contract.
7. With the existing value 23, the stacks after each instruction are
   `[23,9]`, `[23,9,7]`, `[23,16]`, `[23,16,3]`, `[23,16,3,5]`,
   `[23,16,8]`, and `[23,128]`. Peak depth is four after pushing 5.
   The original prefix is preserved and one result is appended. A whole program
   starts empty and must finish with one value, not the two values shown here.
8. The right-nested expression needs three slots; the left-nested expression
   needs two. Each computes `(3*x) mod 256` for all inputs. Associativity of
   modular addition justifies reassociation for these pure, total expressions.
   An effectful language must additionally preserve the required effects and
   evaluation order; floating-point reassociation needs separate numeric rules.
9. `PUSH 99` passes verification but differs from input `x` at `x=0`.
   The verifier establishes target structure and stack safety, not correspondence
   to a given source expression.
10. Folding produces literal zero, compiled as `PUSH 0`. There is no `MUL8`
    for the wrapper to change. The pipeline avoids the broken translation;
    it does not correct its handling of a multiplication that survives folding.
11. Agreement requires `u*v = u+v (mod 256)`, equivalently
    `(u-1)*(v-1) = 1 (mod 256)`. Thus `u-1` must be an odd unit modulo
    256. Every odd residue has one multiplicative inverse, so every even
    u has exactly one even partner v, and odd u has none. There are 128
    ordered pairs. This uses the fact that an integer is invertible modulo
    256 precisely when it is relatively prime to 256, hence odd. The lab
    checks the count and the per-u distribution exhaustively. Multiplication
    of 1 and 1 gives 1 while addition gives 2; zero with zero gives zero
    under either operation.
12. At x=9 the dividend is 16 and the divisor folds to 8, giving 16/8=2.
    At x=250 the dividend is 1, giving 1/8=0 by truncation. The zero is a
    value, not a trap. For `(x+7)/(249+7)`, the divisor is
    (249+7) mod 256=0 for every input, so evaluation traps for all 256
    inputs before and after folding to `div(x+7,0)`. Replacing that node
    with any literal would turn a trapping program into a returning one.
    Checked in `trap_lab.c`.
13. With values 10 and 20 the sum is 30 either way. Left-to-right logs
    [1,2]; right-to-left logs [2,1]. The value claim agrees but the trace
    claim differs. The swap is allowed only when the language permits both
    orders as refinements or when the log is provably unobserved. Checked
    in `trap_lab.c` with explicit sequencing.
14. With b=0 the guarded program takes the false edge and returns 0 without
    dividing. Hoisted division evaluates 10/0 on that path and traps.
    The compiled behavior set then contains a trap absent from the source
    set, violating `B(C(P),i) subset B(P,i)`. The branch guard is a
    correctness condition, not a performance hint.
