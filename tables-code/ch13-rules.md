# Local rules applied in the worked example, each with its guard for wrapping integer arithmetic.

- Source file: `chapters/ch13.tex`
- Original label: `tab:ch13-rules`
- Original caption (verbatim):

> Local rules applied in the worked example, each with its guard for wrapping integer arithmetic.

| Pattern and replacement | Guard | Effect |
| --- | --- | --- |
| x + 0 becomes x | no trapping overflow | removes add |
| x times 1 becomes x | integer type | removes mul |
| x - x becomes 0 | pure integer operand | creates constant |
| x times 0 becomes 0 | pure, no trap | creates constant |
| cmp folded, br folded | constant condition | removes edge |

<!-- Original LaTeX -->

```latex
\begin{tabular}{lcc}
\toprule
Pattern and replacement & Guard & Effect \\
\midrule
x + 0 becomes x & no trapping overflow & removes add \\
x times 1 becomes x & integer type & removes mul \\
x - x becomes 0 & pure integer operand & creates constant \\
x times 0 becomes 0 & pure, no trap & creates constant \\
cmp folded, br folded & constant condition & removes edge \\
\bottomrule
\end{tabular}
```
