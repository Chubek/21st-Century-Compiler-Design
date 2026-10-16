# Dominator sets, back-edge tests, and loop membership for Listing.

- Source file: `chapters/ch18.tex`
- Original label: `tab:ch18-dom-loops`
- Original caption (verbatim):

> Dominator sets, back-edge tests, and loop membership for Listing~\ref{lst:ch18-nested}.

| Block | Dominators | Back edge | Loop membership |
| --- | --- | --- | --- |
| 1 | 1 | none | none |
| 2 | 1, 2 | target of 5 to 2 | outer header |
| 3 | 1, 2, 3 | target of 4 to 3 | outer, inner header |
| 4 | 1, 2, 3, 4 | source of 4 to 3 | outer, inner |
| 5 | 1, 2, 3, 5 | source of 5 to 2 | outer |
| 6 | 1, 2, 6 | none | none |

<!-- Original LaTeX -->

```latex
\begin{tabular}{llll}
    \toprule
    Block & Dominators & Back edge & Loop membership \\
    \midrule
    1 & 1 & none & none \\
    2 & 1, 2 & target of 5 to 2 & outer header \\
    3 & 1, 2, 3 & target of 4 to 3 & outer, inner header \\
    4 & 1, 2, 3, 4 & source of 4 to 3 & outer, inner \\
    5 & 1, 2, 3, 5 & source of 5 to 2 & outer \\
    6 & 1, 2, 6 & none & none \\
    \bottomrule
  \end{tabular}
```
