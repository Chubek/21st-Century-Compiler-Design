# Common loop transformations with their main payoff and the legality check each one requires.

- Source file: `chapters/ch19.tex`
- Original label: `tab:ch19-menu`
- Original caption (verbatim):

> Common loop transformations with their main payoff and the legality check each one requires.

| Transformation | Main payoff | Legality check |
| --- | --- | --- |
| Unrolling | Less overhead, longer bodies | Known count, no early exits |
| Peeling | Aligned steady state | Same conditions for split copies |
| Fusion | Shared data stays hot | No backward dependence across merge |
| Fission | Separated concerns vectorize | No split of a needed ordering |
| Interchange | Sequential memory access | Dependence direction permits swap |
| Tiling | Cache-level reuse | Bands respect all dependences |

<!-- Original LaTeX -->

```latex
\begin{tabular}{lll}
    \toprule
    Transformation & Main payoff & Legality check \\
    \midrule
    Unrolling & Less overhead, longer bodies & Known count, no early exits \\
    Peeling & Aligned steady state & Same conditions for split copies \\
    Fusion & Shared data stays hot & No backward dependence across merge \\
    Fission & Separated concerns vectorize & No split of a needed ordering \\
    Interchange & Sequential memory access & Dependence direction permits swap \\
    Tiling & Cache-level reuse & Bands respect all dependences \\
    \bottomrule
  \end{tabular}
```
