# Each operation reduces its result modulo 256.

- Source file: `chapters/ch01.tex`
- Original label: `tab:tiny-example-values`
- Original caption (verbatim):

> Each operation reduces its result modulo 256.

| Input a | (a+7) mod 256 | (3+5) mod 256 | Final value |
| --- | --- | --- | --- |
| 0 | 7 | 8 | 56 |
| 1 | 8 | 8 | 64 |
| 9 | 16 | 8 | 128 |
| 248 | 255 | 8 | 248 |
| 249 | 0 | 8 | 0 |
| 250 | 1 | 8 | 8 |
| 255 | 6 | 8 | 48 |

<!-- Original LaTeX -->

```latex
\begin{tabular}{rrrr}
    \toprule
    Input $a$ & $(a+7)\bmod256$ & $(3+5)\bmod256$ & Final value \\
    \midrule
    0   & 7   & 8 & 56 \\
    1   & 8   & 8 & 64 \\
    9   & 16  & 8 & 128 \\
    248 & 255 & 8 & 248 \\
    249 & 0   & 8 & 0 \\
    250 & 1   & 8 & 8 \\
    255 & 6   & 8 & 48 \\
    \bottomrule
  \end{tabular}
```
