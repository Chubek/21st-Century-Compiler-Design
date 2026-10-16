# Division traps exactly when its evaluated divisor is zero.

- Source file: `chapters/ch01/traps-effects.tex`
- Original label: `tab:trap-div-cases`
- Original caption (verbatim):

> Division traps exactly when its evaluated divisor is zero.

| Dividend | Divisor | Folded divisor | Outcome |
| --- | --- | --- | --- |
| x+7 at x=9 (16) | 3+5 (8) | 8 | 2 |
| x+7 at x=250 (1) | 3+5 (8) | 8 | 0 |
| x+7 | 249+7 | 0 | trap for every input |
| 7 | 2 | 2 | 3 |
| 0 | 5 | 5 | 0 |

<!-- Original LaTeX -->

```latex
\begin{tabular}{llll}
    \toprule
    Dividend & Divisor & Folded divisor & Outcome \\
    \midrule
    $x+7$ at $x=9$ ($16$) & $3+5$ ($8$) & $8$ & $2$ \\
    $x+7$ at $x=250$ ($1$) & $3+5$ ($8$) & $8$ & $0$ \\
    $x+7$ & $249+7$ & $0$ & $\text{trap}$ for every input \\
    $7$ & $2$ & $2$ & $3$ \\
    $0$ & $5$ & $5$ & $0$ \\
    \bottomrule
  \end{tabular}
```
