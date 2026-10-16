# Tables Code Index

Serialized Markdown copies of all `table` environments in `chapters/` and `appendices/`.
Total tables: 56.

Verification (expected 56, one per `tab:` label, zero without):

```sh
grep -rn 'begin{table}' chapters appendices
grep -rho 'label{tab:[^}]*}' chapters appendices | sort
```

| slug | source | caption |
| --- | --- | --- |
| `app-a-opcodes` | `appendices/appendix-a.tex` | Each opcode has a fixed stack effect. |
| `app-c-ch01` | `appendices/appendix-c.tex` | Set, evaluation, translation, and behavior symbols used in the opening chapter. |
| `app-c-dataflow` | `appendices/appendix-c.tex` | Dataflow macros name boundary facts, generated facts, and lattice operations. |
| `app-c-ssa-cfg` | `appendices/appendix-c.tex` | SSA and control-flow macros name merges, predecessors, dominance, and definitions. |
| `ch02-ast-nodes` | `chapters/ch02.tex` | Abstract syntax tree nodes of Mini with the data each node carries and its evaluation rule. |
| `ch03-token-kinds` | `chapters/ch03.tex` | Token kinds of Mini with example lexemes and recognition rules. |
| `ch04-grammar` | `chapters/ch04.tex` | The Mini grammar with one rule per syntactic category and grouping fixed by rule layering. |
| `ch05-diagnostics` | `chapters/ch05.tex` | Diagnostic kinds in Mini with a triggering example and the message each one produces. |
| `ch06-opcodes` | `chapters/ch06.tex` | Three-address instructions used by Mini lowering with the effect of each opcode. |
| `ch07-blocks` | `chapters/ch07.tex` | Basic blocks of the branching fragment with their instructions and successor blocks. |
| `ch08-domsets` | `chapters/ch08.tex` | Dominator sets for the five-block graph of Figure. |
| `ch09-reaching` | `chapters/ch09.tex` | Reaching definitions on the diamond, shown as entry and exit sets after each pass over the blocks. |
| `ch10-lattices` | `chapters/ch10.tex` | Three small lattices compared by elements, combination operators, and longest chain height. |
| `ch10-trace` | `chapters/ch10.tex` | Sign of `i` at the loop head across Kleene iterates, from the empty start to stabilization at bottom. |
| `ch11-intervals` | `chapters/ch11.tex` | Interval facts for the branch-join prefix with n in [-5, 5], showing how negation and join recover a nonnegative bound. |
| `ch12-df` | `chapters/ch12.tex` | Dominance frontiers and resulting phi placement for `x` and `y` on the loop-and-branch graph. |
| `ch13-rules` | `chapters/ch13.tex` | Local rules applied in the worked example, each with its guard for wrapping integer arithmetic. |
| `ch14-meet` | `chapters/ch14.tex` | Meet table for the flat constant lattice over representative values, read as row meet column. |
| `ch14-trace` | `chapters/ch14.tex` | Lattice states for `d` and `e` at each block exit across two propagation passes. |
| `ch15-patterns` | `chapters/ch15.tex` | Deletion patterns with the safety proof each one requires before removal. |
| `ch16-kinds` | `chapters/ch16.tex` | Three shapes of redundancy differ in where the earlier computation sits and what fix each one needs. |
| `ch16-trace` | `chapters/ch16.tex` | Value numbers assigned while scanning Listing from top to bottom. |
| `ch17-marking` | `chapters/ch17.tex` | Invariant marking over Listing reaches a fixed point after two passes with one member. |
| `ch18-dom-loops` | `chapters/ch18.tex` | Dominator sets, back-edge tests, and loop membership for Listing. |
| `ch19-counts` | `chapters/ch19.tex` | Dynamic operation counts for the accumulation loop rolled and unrolled by four. |
| `ch19-menu` | `chapters/ch19.tex` | Common loop transformations with their main payoff and the legality check each one requires. |
| `ch20-queries` | `chapters/ch20.tex` | Alias answers derived from the fixed point by intersecting target sets. |
| `ch21-avail` | `chapters/ch21.tex` | Available-load sets for Listing with a no-alias answer for the store. |
| `ch21-menu` | `chapters/ch21.tex` | Memory optimizations with the fact that makes each one legal. |
| `ch22-mod` | `chapters/ch22.tex` | May-modify sets for Listing computed bottom-up over the call graph. |
| `ch22-sensitivity` | `chapters/ch22.tex` | Context-sensitivity choices trade caller precision against analysis cost. |
| `ch23-inline-tradeoffs` | `chapters/ch23.tex` | Benefits of inlining and specialization with their main risks. |
| `ch24-convention-choices` | `chapters/ch24.tex` | Calling-convention decisions and what each choice costs. |
| `ch25-pattern-examples` | `chapters/ch25.tex` | Common selection patterns and the IR they absorb. |
| `ch26-scheduler-scope` | `chapters/ch26.tex` | Scheduling scopes with their freedom and main risk. |
| `ch27-alloc-choices` | `chapters/ch27.tex` | Allocator decisions with their payoff and danger. |
| `ch28-destruction-fixes` | `chapters/ch28.tex` | SSA-destruction hazards with their standard repairs. |
| `ch29-reloc-kinds` | `chapters/ch29.tex` | Common relocation computations and when they apply. |
| `ch30-dispatch` | `chapters/ch30.tex` | Dispatch strategies trade portability against branch cost, so handlers should be fixed before dispatch is tuned. |
| `ch31-stack-register` | `chapters/ch31.tex` | Stack and register designs differ in encoding density and dispatch count rather than expressiveness. |
| `ch32-tagging` | `chapters/ch32.tex` | Tagging schemes trade value capacity against test cost and collector complexity. |
| `ch33-collectors` | `chapters/ch33.tex` | Collector designs uphold the same safety rule but pay at different times and fragment differently. |
| `ch34-tiers` | `chapters/ch34.tex` | Tiers trade compile speed against code quality, with profiling connecting each step upward. |
| `ch35-speculations` | `chapters/ch35.tex` | Each speculation pairs an assumed fact with a guard and a tested fallback. |
| `ch36-patterns` | `chapters/ch36.tex` | Loop patterns differ in dependence shape and in their threading verdict. |
| `ch37-oracles` | `chapters/ch37.tex` | Oracles judge results differently, so a suite combines several kinds rather than relying on one. |
| `ch38-ordering` | `chapters/ch38.tex` | Ordering rules pair each sequence constraint with the defect it prevents. |
| `ch38-threats` | `chapters/ch38.tex` | Common measurement threats pair each risk with its mitigation in the checklist. |
| `tiny-example-values` | `chapters/ch01.tex` | Each operation reduces its result modulo 256. |
| `tiny-reduction-steps` | `chapters/ch01/fault-lab.tex` | Five accepted replacements retain the wrong-code failure at input 9. |
| `tiny-stack-trace` | `chapters/ch01/stack-translation.tex` | The seven-instruction program produces 8 at input 250. |
| `trap-div-cases` | `chapters/ch01/traps-effects.tex` | Division traps exactly when its evaluated divisor is zero. |
| `ch39-rules` | `chapters/ch39.tex` | Typing rules for Mini-Fun. |
| `app-d-correspondence` | `appendices/appendix-d.tex` | The working correspondence used by the book. |
| `ch06-ir-choices` | `chapters/ch06.tex` | Intermediate-form choices with what each one makes explicit and what it costs. |
| `ch34-bbv-limits` | `chapters/ch34.tex` | Version-budget choices with their payoff and danger. |

Each `tables-code/<slug>.md` contains: title, metadata block (source, label, verbatim caption blockquote), GFM table, and trailing `<!-- Original LaTeX -->` fenced `latex` block with the exact `tabular` verbatim.
