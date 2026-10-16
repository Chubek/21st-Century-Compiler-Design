# Deletion patterns with the safety proof each one requires before removal.

- Source file: `chapters/ch15.tex`
- Original label: `tab:ch15-patterns`
- Original caption (verbatim):

> Deletion patterns with the safety proof each one requires before removal.

| Pattern | Safety proof needed | Action |
| --- | --- | --- |
| Pure def, no live use | liveness fixed point | delete definition |
| Dead branch arm | constant condition | fold to jump |
| Unreachable block | no live control path | delete block |
| Effectful or trapping op | no pureness proof | keep operation |

<!-- Original LaTeX -->

```latex
\begin{tabular}{lcc}
\toprule
Pattern & Safety proof needed & Action \\
\midrule
Pure def, no live use & liveness fixed point & delete definition \\
Dead branch arm & constant condition & fold to jump \\
Unreachable block & no live control path & delete block \\
Effectful or trapping op & no pureness proof & keep operation \\
\bottomrule
\end{tabular}
```
