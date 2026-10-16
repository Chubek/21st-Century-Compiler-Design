# Dynamic operation counts for the accumulation loop rolled and unrolled by four.

- Source file: `chapters/ch19.tex`
- Original label: `tab:ch19-counts`
- Original caption (verbatim):

> Dynamic operation counts for the accumulation loop rolled and unrolled by four.

| Case | Tests plus updates | Loads plus adds | Body copies |
| --- | --- | --- | --- |
| Rolled, T = 100 | 201 | 200 | 1 |
| Unrolled k = 4, T = 100 | 51 | 200 | 4 |
| Unrolled k = 4, T = 102 | 56 including cleanup | 204 | 4 plus cleanup |

<!-- Original LaTeX -->

```latex
\begin{tabular}{llll}
    \toprule
    Case & Tests plus updates & Loads plus adds & Body copies \\
    \midrule
    Rolled, $T = 100$ & 201 & 200 & 1 \\
    Unrolled $k = 4$, $T = 100$ & 51 & 200 & 4 \\
    Unrolled $k = 4$, $T = 102$ & 56 including cleanup & 204 & 4 plus cleanup \\
    \bottomrule
  \end{tabular}
```
