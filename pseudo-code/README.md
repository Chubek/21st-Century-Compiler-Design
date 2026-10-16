# Serialized pseudocode

Plain-text serializations of every `algorithm` environment in `chapters/` and `appendices/`.

Conventions: one step per line, 2-space indent for nesting, uppercase keywords
(REQUIRE/ENSURE/FUNCTION/FOR/FORALL/WHILE/IF/ELSE/RETURN/BREAK/CONTINUE/REPEAT/UNTIL),
math macros expanded to ASCII (e.g. `\In{B}` -> `In[B]`, `\meet` -> `MEET`).
Each `.sudo` ends with the verbatim LaTeX body as `# `-prefixed comments.

| slug | source file | caption | label |
| ---- | ----------- | ------- | ----- |
| tiny-fold | chapters/ch01.tex:339 | Fold constant operations in a finite expression tree. | alg:tiny-fold |
| ch02-eval | chapters/ch02.tex:123 | Tree-walking evaluation of Mini statements and expressions. | alg:ch02-eval |
| ch03-scan | chapters/ch03.tex:115 | Maximal-munch scanning of Mini source text. | alg:ch03-scan |
| ch04-parse | chapters/ch04.tex:86 | Recursive-descent parsing of Mini expressions with layered precedence. | alg:ch04-parse |
| ch05-check | chapters/ch05.tex:83 | Single-pass name resolution and type checking for Mini. | alg:ch05-check |
| ch06-lower | chapters/ch06.tex:94 | Lowering Mini expressions to three-address code. | alg:ch06-lower |
| ch07-cfg | chapters/ch07.tex:97 | Leader-based construction of the control-flow graph. | alg:ch07-cfg |
| ch08-dom | chapters/ch08.tex:70 | Iterative computation of dominator sets. | alg:ch08-dom |
| ch09-worklist | chapters/ch09.tex:142 | Worklist iterative dataflow for gen-kill problems | alg:ch09-worklist |
| ch10-fixedpoint | chapters/ch10.tex:88 | Generic lattice fixed-point iteration with a worklist | alg:ch10-fixedpoint |
| ch11-abstract | chapters/ch11.tex:86 | Abstract interpretation over a CFG with widening at loop heads | alg:ch11-abstract |
| ch12-placement | chapters/ch12.tex:68 | Minimal phi placement by iterated dominance frontiers | alg:ch12-placement |
| ch13-simplify | chapters/ch13.tex:78 | Local simplification to a normal form within one block | alg:ch13-simplify |
| ch14-constprop | chapters/ch14.tex:66 | Forward constant propagation on the flat lattice | alg:ch14-constprop |
| ch15-adce | chapters/ch15.tex:71 | Conservative marking-based dead-code elimination | alg:ch15-adce |
| ch16-lvn | chapters/ch16.tex:78 | Local value numbering with current value holders | alg:ch16-lvn |
| ch17-licm | chapters/ch17.tex:58 | Loop-invariant code motion to the preheader | alg:ch17-licm |
| ch18-natural-loops | chapters/ch18.tex:55 | Natural loop discovery from back edges | alg:ch18-natural-loops |
| ch19-unroll | chapters/ch19.tex:58 | Unrolling a counted loop with a cleanup loop | alg:ch19-unroll |
| ch20-points-to | chapters/ch20.tex:76 | Worklist solver for inclusion points-to constraints | alg:ch20-points-to |
| ch21-load-elim | chapters/ch21.tex:47 | Redundant load elimination with alias kills | alg:ch21-load-elim |
| ch22-maymod | chapters/ch22.tex:76 | May-modify summaries over the call graph | alg:ch22-maymod |
| ch23-inline | chapters/ch23.tex:125 | Greedy cost-guided inlining with a size budget | alg:ch23-inline |
| ch24-lower-call | chapters/ch24.tex:111 | Lower a call and fix the enclosing frame | alg:ch24-lower-call |
| ch25-tree-dp | chapters/ch25.tex:100 | Tree-pattern instruction selection by dynamic programming | alg:ch25-tree-dp |
| ch26-list | chapters/ch26.tex:93 | Forward list scheduling for one region | alg:ch26-list |
| ch27-coloring | chapters/ch27.tex:97 | Graph-coloring allocation with spill rewrite | alg:ch27-coloring |
| ch28-out-of-ssa | chapters/ch28.tex:124 | Out-of-SSA translation with edge splitting | alg:ch28-out-of-ssa |
| ch29-link | chapters/ch29.tex:89 | Static link: resolve, lay out, and relocate | alg:ch29-link |
| ch30-bytecode-loop | chapters/ch30.tex:118 | Bytecode dispatch loop with explicit errors and fuel | alg:ch30-bytecode-loop |
| ch31-verify | chapters/ch31.tex:129 | Stack-depth verification over a bytecode control-flow graph | alg:ch31-verify |
| ch32-layout | chapters/ch32.tex:83 | Field offset assignment for single-inheritance classes | alg:ch32-layout |
| ch33-mark | chapters/ch33.tex:89 | Tri-color marking with an explicit worklist | alg:ch33-mark |
| ch34-tier-up | chapters/ch34.tex:79 | Tier promotion from counters and profile maturity | alg:ch34-tier-up |
| ch35-deopt | chapters/ch35.tex:85 | Deoptimization after a failed guard | alg:ch35-deopt |
| ch36-vector-check | chapters/ch36.tex:81 | Legality check for vectorizing an affine loop nest | alg:ch36-vector-check |
| ch37-reduce | chapters/ch37.tex:85 | Greedy single-fragment test-case reduction | alg:ch37-reduce |
| ch38-pipeline-run | chapters/ch38.tex:80 | Contract-checked pipeline driver with fact versions | alg:ch38-pipeline-run |
| tiny-greedy-reduce | chapters/ch01/fault-lab.tex:136 | Reduce a failing tree using a fixed set of local replacements. | alg:tiny-greedy-reduce |
| tiny-stack-emit | chapters/ch01/stack-translation.tex:95 | Emit stack instructions for an expression. | alg:tiny-stack-emit |
| trap-guarded-fold | chapters/ch01/traps-effects.tex:278 | Fold constants in the presence of a trapping division. | alg:trap-guarded-fold |
| ch39-check | chapters/ch39.tex:82 | Syntax-directed checking for Mini-Fun. | alg:ch39-check |
| app-d-subst | appendices/appendix-d.tex:22 | Capture-avoiding substitution of $N$ for $x$ in $M$. | alg:app-d-subst |
| ch34-bbv | chapters/ch34.tex:140 | Lazy basic block versioning under a version budget. | alg:ch34-bbv |
