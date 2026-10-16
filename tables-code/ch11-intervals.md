# Interval facts for the branch-join prefix with n in [-5, 5], showing how negation and join recover a nonnegative bound.

- Source file: `chapters/ch11.tex`
- Original label: `tab:ch11-intervals`
- Original caption (verbatim):

> Interval facts for the branch-join prefix with $n$ in $[-5, 5]$, showing how negation and join recover a nonnegative bound.

| Block | Abstract entry state | Abstract exit state |
| --- | --- | --- |
| B1 | n in [-5, 5] | n in [-5, 5] |
| B2 | n in [-5, -1] | x in [1, 5] |
| B3 | n in [0, 5] | x in [0, 5] |
| B4 | x in [0, 5] | i in [0, 0], x in [0, 5] |

<!-- Original LaTeX -->

```latex
\begin{tabular}{lcc}
\toprule
Block & Abstract entry state & Abstract exit state \\
\midrule
B1 & n in [-5, 5] & n in [-5, 5] \\
B2 & n in [-5, -1] & x in [1, 5] \\
B3 & n in [0, 5] & x in [0, 5] \\
B4 & x in [0, 5] & i in [0, 0], x in [0, 5] \\
\bottomrule
\end{tabular}
```
