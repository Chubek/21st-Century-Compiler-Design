# Dominance frontiers and resulting phi placement for `x` and `y` on the loop-and-branch graph.

- Source file: `chapters/ch12.tex`
- Original label: `tab:ch12-df`
- Original caption (verbatim):

> Dominance frontiers and resulting phi placement for \code{x} and \code{y} on the loop-and-branch graph.

| Block | Frontier set | Phi placed |
| --- | --- | --- |
| B1 | empty | none |
| B2 | loop head B2 | phi for x at B2 |
| B3 | join B5 | phi for x at B5 |
| B4 | join B5 | phi already at B5 |
| B5 | loop head B2 | none new |

<!-- Original LaTeX -->

```latex
\begin{tabular}{lcc}
\toprule
Block & Frontier set & Phi placed \\
\midrule
B1 & empty & none \\
B2 & loop head B2 & phi for x at B2 \\
B3 & join B5 & phi for x at B5 \\
B4 & join B5 & phi already at B5 \\
B5 & loop head B2 & none new \\
\bottomrule
\end{tabular}
```
