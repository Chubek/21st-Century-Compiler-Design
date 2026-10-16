# Basic blocks of the branching fragment with their instructions and successor blocks.

- Source file: `chapters/ch07.tex`
- Original label: `tab:ch07-blocks`
- Original caption (verbatim):

> Basic blocks of the branching fragment with their instructions and successor blocks.

| Block | Instructions | Successors |
| --- | --- | --- |
| B1 | test and conditional branch | B2, B3 |
| B2 | assign `y = 1` and jump | B4 |
| B3 | assign `y = 2`, falls through | B4 |
| B4 | print `y` | none, exit |

<!-- Original LaTeX -->

```latex
\begin{tabular}{lll}
\toprule
Block & Instructions & Successors \\
\midrule
B1 & test and conditional branch & B2, B3 \\
B2 & assign \code{y = 1} and jump & B4 \\
B3 & assign \code{y = 2}, falls through & B4 \\
B4 & print \code{y} & none, exit \\
\bottomrule
\end{tabular}
```
