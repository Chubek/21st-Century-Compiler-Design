# Context-sensitivity choices trade caller precision against analysis cost.

- Source file: `chapters/ch22.tex`
- Original label: `tab:ch22-sensitivity`
- Original caption (verbatim):

> Context-sensitivity choices trade caller precision against analysis cost.

| Choice | Precision | Cost driver |
| --- | --- | --- |
| Insensitive | Merged over callers | One summary per procedure |
| Cloning | Selected contexts separated | Copies scale with contexts |
| Summaries | Per-caller instantiation | Summary size and instantiation |

<!-- Original LaTeX -->

```latex
\begin{tabular}{lll}
    \toprule
    Choice & Precision & Cost driver \\
    \midrule
    Insensitive & Merged over callers & One summary per procedure \\
    Cloning & Selected contexts separated & Copies scale with contexts \\
    Summaries & Per-caller instantiation & Summary size and instantiation \\
    \bottomrule
  \end{tabular}
```
