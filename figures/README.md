# Visual companions

These TikZ sources are included by the chapter that explains them. Each file
contains a captioned, labeled figure; its chapter supplies the reference and
interpretation. The diagrams use the book's existing fonts and graph styles.
Grayscale fills, text labels, and solid/dashed lines carry the distinctions.

| Source | Location | Reading task |
| --- | --- | --- |
| `lexeme-spans.tex` | Chapter 3 | Follow half-open character spans and the scanner cursor. |
| `dataflow-merge.tex` | Chapter 9 | See which reaching definitions survive a union. |
| `ssa-dominator-tree.tex` | Chapter 12 | Distinguish dominator-tree traversal from execution edges. |
| `unroll-tail.tex` | Chapter 19 | Check coverage of full groups and the scalar tail. |
| `schedule-timeline.tex` | Chapter 26 | Separate issue slots from result latencies. |
| `parallel-copy.tex` | Chapter 28 | Separate source reads from destination writes. |
| `stack-arity.tex` | Chapter 31 | Detect underflow that net depth alone misses. |
| `object-byte-map.tex` | Chapter 32 | Locate fields and padding at proportional byte offsets. |
| `gc-color-transition.tex` | Chapter 33 | Preserve the pending worklist while scanning one object. |
| `deopt-frame-map.tex` | Chapter 35 | Recover live values at the correct semantic boundary. |
| `subset-lattice.tex` | Appendix C | Read meet and join under an explicitly stated order. |

Four C listings in Chapters 3, 28, 31, and 32 are included directly from
[`example-code/visual-labs/`](../example-code/visual-labs/README.md).
The examples are deliberately bounded components with documented failure
contracts, not complete implementations of the surrounding compiler passes.

Build the complete manuscript with `texlua build.lua`. The existing lexer,
unrolling, and critical-edge diagrams were also redrawn during this pass to
remove label overlaps and clarify the actual graph edges.
