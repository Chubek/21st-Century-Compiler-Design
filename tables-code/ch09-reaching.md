# Reaching definitions on the diamond, shown as entry and exit sets after each pass over the blocks.

- Source file: `chapters/ch09.tex`
- Original label: `tab:ch09-reaching`
- Original caption (verbatim):

> Reaching definitions on the diamond, shown as entry and exit sets after each pass over the blocks.

| Block | Pass 1 entry and exit | Pass 2 entry and exit |
| --- | --- | --- |
| B1 | In empty, Out empty | In empty, Out empty |
| B2 | In empty, Out d1 d2 | In empty, Out d1 d2 |
| B3 | In empty, Out d3 d4 | In empty, Out d3 d4 |
| B4 | In d1 d2 d3 d4, Out same | In d1 d2 d3 d4, Out same |

<!-- Original LaTeX -->

```latex
\begin{tabular}{lcc}
\toprule
Block & Pass 1 entry and exit & Pass 2 entry and exit \\
\midrule
B1 & In empty, Out empty & In empty, Out empty \\
B2 & In empty, Out d1 d2 & In empty, Out d1 d2 \\
B3 & In empty, Out d3 d4 & In empty, Out d3 d4 \\
B4 & In d1 d2 d3 d4, Out same & In d1 d2 d3 d4, Out same \\
\bottomrule
\end{tabular}
```
