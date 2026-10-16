# Dispatch strategies trade portability against branch cost, so handlers should be fixed before dispatch is tuned.

- Source file: `chapters/ch30.tex`
- Original label: `tab:ch30-dispatch`
- Original caption (verbatim):

> Dispatch strategies trade portability against branch cost, so handlers should be fixed before dispatch is tuned.

| Strategy | Portability | Effort | Branch behavior |
| --- | --- | --- | --- |
| Tree switch | High | Low | One branch per node type |
| Loop switch | High | Low | One branch per opcode |
| Computed jump | Medium | Medium | Indirect branch per opcode |
| Direct threading | Low | Medium | Chained handler jumps |
| Call threading | Medium | High | Native calls to handlers |

<!-- Original LaTeX -->

```latex
\begin{tabular}{llll}
\toprule
Strategy & Portability & Effort & Branch behavior \\
\midrule
Tree switch & High & Low & One branch per node type \\
Loop switch & High & Low & One branch per opcode \\
Computed jump & Medium & Medium & Indirect branch per opcode \\
Direct threading & Low & Medium & Chained handler jumps \\
Call threading & Medium & High & Native calls to handlers \\
\bottomrule
\end{tabular}
```
