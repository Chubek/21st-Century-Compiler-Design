# SSA and control-flow macros name merges, predecessors, dominance, and definitions.

- Source file: `appendices/appendix-c.tex`
- Original label: `tab:app-c-ssa-cfg`
- Original caption (verbatim):

> SSA and control-flow macros name merges, predecessors, dominance, and definitions.

| Macro | Meaning |
| --- | --- |
| `\phifn{...}` | merge of values from predecessor blocks |
| `\assignphi` | assignment of a merged value to a name |
| `\Pred` | set of CFG predecessors of a block |
| `\Succ` | set of CFG successors of a block |
| `\dom` | set of blocks dominating a block |
| `\idom` | immediate dominator of a block |
| `\DF` | dominance frontier of a block |
| `\Def` | set of definitions of a variable |
| `\Use` | set of uses of a variable |
| `\Entry` | distinguished CFG entry block |
| `\Exit` | distinguished CFG exit block |

<!-- Original LaTeX -->

```latex
\begin{tabular}{ll}
    \toprule
    Macro & Meaning \\
    \midrule
    \texttt{\textbackslash phifn\{...\}} & merge of values from predecessor blocks \\
    \texttt{\textbackslash assignphi} & assignment of a merged value to a name \\
    \texttt{\textbackslash Pred} & set of CFG predecessors of a block \\
    \texttt{\textbackslash Succ} & set of CFG successors of a block \\
    \texttt{\textbackslash dom} & set of blocks dominating a block \\
    \texttt{\textbackslash idom} & immediate dominator of a block \\
    \texttt{\textbackslash DF} & dominance frontier of a block \\
    \texttt{\textbackslash Def} & set of definitions of a variable \\
    \texttt{\textbackslash Use} & set of uses of a variable \\
    \texttt{\textbackslash Entry} & distinguished CFG entry block \\
    \texttt{\textbackslash Exit} & distinguished CFG exit block \\
    \bottomrule
  \end{tabular}
```
