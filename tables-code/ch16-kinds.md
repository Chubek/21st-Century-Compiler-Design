# Three shapes of redundancy differ in where the earlier computation sits and what fix each one needs.

- Source file: `chapters/ch16.tex`
- Original label: `tab:ch16-kinds`
- Original caption (verbatim):

> Three shapes of redundancy differ in where the earlier computation sits and what fix each one needs.

| Shape | Where the duplicate sits | Remedy |
| --- | --- | --- |
| Local | Same basic block | Copy from the first computation |
| Global | Every path to the use | Dataflow availability plus reuse |
| Partial | Some paths to the use | Insert on missing paths, then reuse |

<!-- Original LaTeX -->

```latex
\begin{tabular}{lll}
    \toprule
    Shape & Where the duplicate sits & Remedy \\
    \midrule
    Local & Same basic block & Copy from the first computation \\
    Global & Every path to the use & Dataflow availability plus reuse \\
    Partial & Some paths to the use & Insert on missing paths, then reuse \\
    \bottomrule
  \end{tabular}
```
