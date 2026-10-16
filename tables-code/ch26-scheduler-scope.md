# Scheduling scopes with their freedom and main risk.

- Source file: `chapters/ch26.tex`
- Original label: `tab:ch26-scheduler-scope`
- Original caption (verbatim):

> Scheduling scopes with their freedom and main risk.

| Scope | Freedom | Main risk |
| --- | --- | --- |
| Basic block | Local reorder only | Small hiding window |
| Superblock and trace | Cross-branch motion | Compensation code |
| Software pipeline | Cross-iteration overlap | Code growth, pressure |
| Pre-allocation | Full renaming room | Pressure blowup |
| Post-allocation | Exact latencies known | False dependences stuck |

<!-- Original LaTeX -->

```latex
\begin{tabular}{lll}
\toprule
Scope & Freedom & Main risk \\
\midrule
Basic block & Local reorder only & Small hiding window \\
Superblock and trace & Cross-branch motion & Compensation code \\
Software pipeline & Cross-iteration overlap & Code growth, pressure \\
Pre-allocation & Full renaming room & Pressure blowup \\
Post-allocation & Exact latencies known & False dependences stuck \\
\bottomrule
\end{tabular}
```
