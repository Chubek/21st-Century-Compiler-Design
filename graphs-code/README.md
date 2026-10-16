# graphs-code — serialized TikZ diagrams

Verbatim TikZ extractions for every `tikzpicture` in the book.
Each `graphs-code/<slug>.tikz.tex` holds exactly one `tikzpicture` environment,
copied verbatim from its source, plus leading `%` comments with provenance.

Conventions:

- `<slug>` is the figure label without the `fig:` prefix
  (e.g. `\label{fig:ch07-diamond-loop}` → `graphs-code/ch07-diamond-loop.tikz.tex`).
- For `figures/*.tex` inputs the label lives in the same file.
- For inline chapter pictures the slug comes from the nearest enclosing
  `figure` environment (the `\label{fig:...}` within that environment).
- No picture was found without an enclosing `fig:` label, so no `<file>-inline-<n>` fallbacks were needed.
- Bodies are byte-identical to the source tikzpicture blocks (begin/end pair)
  (verified: each source block appears verbatim in its output file).
- Files contain no `\documentclass` / `\begin{document}`; they are meant to be
  `\input` inside a `figure` environment in the book.
- Required TikZ setup is `support/compiler-pseudocode.sty`, which loads
  `tikz` + `arrows.meta, positioning, shapes.geometric, fit` and defines
  styles `basic block`, `cfg edge`, `back edge`, `block label`.
  Per-file headers list which shared styles each picture actually uses.

Total: **58** pictures (source grep for tikzpicture begins in chapters/figures/appendices → 58;
`grep -rho 'label{fig:[^}]*}' chapters figures appendices | sort` → 58 labels, 1:1 match).

Verify with:

```sh
grep -rn 'begin{tikzpicture}' chapters figures appendices
grep -rho 'label{fig:[^}]*}' chapters figures appendices | sort
grep -rn 'begin{tikzpicture}' graphs-code --include="*.tikz.tex" | wc -l
```

## Index

| slug | source | caption | label |
| --- | --- | --- | --- |
| `ch02-eval-while` | `chapters/ch02.tex:98` | Evaluation order for a while loop uses three steps and one repeated edge that returns to the test. | `fig:ch02-eval-while` |
| `ch03-word-dfa` | `chapters/ch03.tex:70` | Word scanning stays in one state while word characters continue and emits a token when anything else appears. | `fig:ch03-word-dfa` |
| `ch04-parse-tree` | `chapters/ch04.tex:60` | The parse tree for \code{2 + 3 * 4} groups the multiplication first, so evaluation yields $14$. | `fig:ch04-parse-tree` |
| `ch05-scope-chain` | `chapters/ch05.tex:60` | Resolving the inner use of \code{x} finds the nearest binding first and consults the outer scope only on a miss. | `fig:ch05-scope-chain` |
| `ch06-lowering-stages` | `chapters/ch06.tex:65` | Lowering flattens the checked tree into one list and then splits the list at labels into blocks. | `fig:ch06-lowering-stages` |
| `ch07-diamond-loop` | `chapters/ch07.tex:74` | The diamond test with both arms joining at the print block and a dashed edge looping back to the test. | `fig:ch07-diamond-loop` |
| `ch08-cfg-domtree` | `chapters/ch08.tex:23` | A five-block loop graph whose head dominates both arms while neither arm dominates the other. | `fig:ch08-cfg-domtree` |
| `ch09-diamond` | `chapters/ch09.tex:201` | The diamond control-flow graph used for hand computation, where $B1$ branches to $B2$ and $B3$ and both join at $B4$. | `fig:ch09-diamond` |
| `ch10-loop` | `chapters/ch10.tex:134` | A counting loop whose head merges the first-entry value with values carried around the back edge, which forces the sign of \code{i} to bottom. | `fig:ch10-loop` |
| `ch11-branch-loop` | `chapters/ch11.tex:140` | Branch followed by a loop, where the join at $B4$ must recover nonnegativity of \code{x} before the loop head at $B5$ analyzes the bound. | `fig:ch11-branch-loop` |
| `ch12-ssa` | `chapters/ch12.tex:122` | A branch inside a loop, where \code{x} needs phis at both the branch join $B5$ and the loop head $B2$ after frontier iteration. | `fig:ch12-ssa` |
| `ch13-block` | `chapters/ch13.tex:121` | The single simplifiable block $B1$ with two successors, where folding the branch to $B2$ later deadens the edge to $B3$. | `fig:ch13-block` |
| `ch14-diamond` | `chapters/ch14.tex:120` | The constant-propagation diamond where \code{d} agrees at $7$ on both arms and \code{e} disagrees, so only \code{d} folds at the join. | `fig:ch14-diamond` |
| `ch15-cfg` | `chapters/ch15.tex:118` | The dead-code diamond where \code{t} in $B2$ is dead but both definitions of \code{u} stay live through the use in $B4$. | `fig:ch15-cfg` |
| `ch16-join` | `chapters/ch16.tex:172` | The join block recomputes a sum available on every path, so it is fully redundant once the branches reuse the entry value. | `fig:ch16-join` |
| `ch17-hoist` | `chapters/ch17.tex:127` | Hoisting moves the invariant product into the preheader, so the loop body keeps only the variant work. | `fig:ch17-hoist` |
| `ch18-nested` | `chapters/ch18.tex:127` | The inner loop sits inside the outer loop, and each back edge returns to the header that dominates its source. | `fig:ch18-nested` |
| `ch19-unrolled-shape` | `chapters/ch19.tex:121` | Unrolling by four keeps the loop skeleton and quadruples the body, so one test governs four loads and four additions. | `fig:ch19-unrolled-shape` |
| `ch20-merge` | `chapters/ch20.tex:150` | The join block sees both bindings of the pointer, so its target set is the union of the two branches. | `fig:ch20-merge` |
| `ch21-diamond` | `chapters/ch21.tex:127` | The join load is redundant only when the store cannot reach the loaded address, which is exactly what the alias oracle decides. | `fig:ch21-diamond` |
| `ch22-call-graph` | `chapters/ch22.tex:52` | Direct calls give an exact acyclic call graph, so summaries propagate bottom-up from the leaves. | `fig:ch22-call-graph` |
| `ch23-inline-scope` | `chapters/ch23.tex:60` | Inlining merges the callee body into the caller, so later passes see one region instead of two isolated ones. | `fig:ch23-inline-scope` |
| `ch24-frame` | `chapters/ch24.tex:54` | A canonical stack frame grows from saved state through spills to outgoing argument space, with alignment padding inserted where the ABI requires it. | `fig:ch24-frame` |
| `ch25-tiling` | `chapters/ch25.tex:36` | The multiply and add form an address subtree. When consumed by a load, the whole fragment may match a scaled-index load tile. | `fig:ch25-tiling` |
| `ch26-dag` | `chapters/ch26.tex:47` | The load feeds the add while the multiply stays independent, so the scheduler may place the multiply between them without violating any edge. | `fig:ch26-dag` |
| `ch27-interference` | `chapters/ch27.tex:47` | Interference edges join values that are live together, so with two registers this graph forces one spill or split around the tight region. | `fig:ch27-interference` |
| `ch28-critical` | `chapters/ch28.tex:64` | Splitting $A\to J$ puts its phi copies in $S$, so they run neither on $A\to Z$ nor on $C\to J$. | `fig:ch28-critical` |
| `ch29-merge` | `chapters/ch29.tex:40` | The linker concatenates matching input sections into output segments, resolves symbols across objects, and patches each relocation against the final layout. | `fig:ch29-merge` |
| `ch30-eval-flow` | `chapters/ch30.tex:60` | Evaluation of one assignment flows from the statement node to operand lookups and then to a single environment update, which keeps resolution and mutation in one place. | `fig:ch30-eval-flow` |
| `ch31-machine-state` | `chapters/ch31.tex:56` | Machine state separates control, temporaries, activation records, and heap data so each instruction touches a defined region. | `fig:ch31-machine-state` |
| `ch32-tagged-value` | `chapters/ch32.tex:35` | A 64-bit tagged value keeps its kind in two low bits and its content in the remaining payload, so a type test is one mask and one compare. | `fig:ch32-tagged-value` |
| `ch33-heap-graph` | `chapters/ch33.tex:29` | Two roots reach A, B, C, and D through shared and direct edges, while E and F have no path from any root and are garbage. | `fig:ch33-heap-graph` |
| `ch34-tiers` | `chapters/ch34.tex:30` | Tiered execution moves hot code upward as counters grow and returns through deoptimization when assumptions fail, so profiling pays only where it matters. | `fig:ch34-tiers` |
| `ch35-guard-point` | `chapters/ch35.tex:33` | Each guarded operation on the fast path can reach a shared exit stub that rebuilds interpreter state, so no bet is placed without a tested way back. | `fig:ch35-guard-point` |
| `ch36-lanes` | `chapters/ch36.tex:34` | Scalar code steps through elements one at a time while vector code covers four lanes per step, which pays only when lanes are independent. | `fig:ch36-lanes` |
| `ch37-pipeline` | `chapters/ch37.tex:30` | Contract checks between passes turn a silent miscompilation into a located failure by testing each handoff before the next pass runs. | `fig:ch37-pipeline` |
| `ch38-pipeline` | `chapters/ch38.tex:32` | Passes flow from language form through optimization to machine form with contract checks at each handoff, so ordering defects surface where they occur. | `fig:ch38-pipeline` |
| `first-diamond` | `chapters/ch01.tex:689` | A branch selects one computation; the return block is shared. | `fig:first-diamond` |
| `first-pipeline` | `chapters/ch01.tex:783` | Different representations make different obligations explicit. | `fig:first-pipeline` |
| `tiny-before-after` | `chapters/ch01.tex:202` | Folding the right addition leaves the input-dependent subtree unchanged. | `fig:tiny-before-after` |
| `tiny-fault-diagnosis` | `chapters/ch01/fault-lab.tex:237` | The verifier accepts both programs; only one preserves the source value. | `fig:tiny-fault-diagnosis` |
| `tiny-stack-boundary` | `chapters/ch01/stack-translation.tex:151` | Correct translation connects two execution mechanisms at a common result. | `fig:tiny-stack-boundary` |
| `trap-hoist` | `chapters/ch01/traps-effects.tex:231` | Hoisting a risky operation above its guard adds behaviors the source lacked. | `fig:trap-hoist` |
| `visual-copy-snapshot` | `figures/parallel-copy.tex:3` | Saving both sources makes a swap independent of destination-write order; the dashed boundary separates the read and write phases. | `fig:visual-copy-snapshot` |
| `visual-deopt-map` | `figures/deopt-frame-map.tex:3` | A deoptimization descriptor maps live compiled values into an interpreter state at the same semantic point, before the guarded addition. | `fig:visual-deopt-map` |
| `visual-gc-transition` | `figures/gc-color-transition.tex:3` | Marking moves the scanned object to black and its newly discovered children to gray without discarding other pending roots. | `fig:visual-gc-transition` |
| `visual-lexeme-spans` | `figures/lexeme-spans.tex:3` | Half-open source spans distinguish three tokens in five characters; the final cursor points just past the input. | `fig:visual-lexeme-spans` |
| `visual-object-byte-map` | `figures/object-byte-map.tex:3` | The 40-byte object contains four internal padding bytes and seven trailing padding bytes; widths are proportional to byte counts. | `fig:visual-object-byte-map` |
| `visual-reaching-union` | `figures/dataflow-merge.tex:3` | A may-analysis retains the definitions from either incoming path; the use-only join block passes them through unchanged. | `fig:visual-reaching-union` |
| `visual-schedule-timeline` | `figures/schedule-timeline.tex:3` | Moving the independent add $c$ changes where the machine idles but cannot shorten the load--multiply--multiply--store dependence chain. | `fig:visual-schedule-timeline` |
| `visual-ssa-domtree` | `figures/ssa-dominator-tree.tex:3` | The dominator-tree walk visits the exit while the join's version $x_5$ is still active; it does not restore the pre-loop version first. | `fig:visual-ssa-domtree` |
| `visual-stack-arity` | `figures/stack-arity.tex:3` | An add with only one operand fails even though its net depth would be zero; the verifier must check pops before pushes. | `fig:visual-stack-arity` |
| `visual-subset-lattice` | `figures/subset-lattice.tex:3` | For subsets ordered by inclusion, upward arrows add possibilities, join is union, and meet is intersection. | `fig:visual-subset-lattice` |
| `visual-unroll-tail` | `figures/unroll-tail.tex:3` | Two full groups and a two-iteration tail cover ten iterations once each, preserving their original order. | `fig:visual-unroll-tail` |
| `ch39-derivation` | `chapters/ch39.tex:61` | The derivation for a one-argument increment function reads upward. | `fig:ch39-derivation` |
| `app-d-beta` | `appendices/appendix-d.tex:49` | One beta step contracts the marked application and returns the argument unchanged. | `fig:app-d-beta` |
| `ch06-ir-tower` | `chapters/ch06.tex:201` | Each stage makes one structure explicit: order, then control joins, then one-definition naming. | `fig:ch06-ir-tower` |
| `ch34-bbv-versions` | `chapters/ch34.tex:164` | Versioning splits one body into an unchecked path and a guarded path. | `fig:ch34-bbv-versions` |

## Notes

- `appendices/appendix-d.tex` contributes one picture (`app-d-beta`).
- Inline fragments `chapters/ch01/stack-translation.tex`, `chapters/ch01/fault-lab.tex`,
  `chapters/ch01/traps-effects.tex` each contribute one picture (included via the ch01 flow).
- `figures/*.tex` each contribute one picture wrapped in their own `figure` environment.
- Unlabeled pictures: **0**. Every `tikzpicture` had an enclosing `fig:` label within its `figure` env.
