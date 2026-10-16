# May-modify sets for Listing computed bottom-up over the call graph.

- Source file: `chapters/ch22.tex`
- Original label: `tab:ch22-mod`
- Original caption (verbatim):

> May-modify sets for Listing~\ref{lst:ch22-calls} computed bottom-up over the call graph.

| Procedure | Direct stores | Callees add | Final summary |
| --- | --- | --- | --- |
| h | g1 | none | {g1} |
| g | g2 | g1 from h | {g1, g2} |
| f | none | g1, g2 from g | {g1, g2} |
| main | none | g1, g2 from f | {g1, g2} |

<!-- Original LaTeX -->

```latex
\begin{tabular}{llll}
    \toprule
    Procedure & Direct stores & Callees add & Final summary \\
    \midrule
    $h$ & $g1$ & none & $\{g1\}$ \\
    $g$ & $g2$ & $g1$ from $h$ & $\{g1, g2\}$ \\
    $f$ & none & $g1, g2$ from $g$ & $\{g1, g2\}$ \\
    $main$ & none & $g1, g2$ from $f$ & $\{g1, g2\}$ \\
    \bottomrule
  \end{tabular}
```
