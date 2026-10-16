# SSA-destruction hazards with their standard repairs.

- Source file: `chapters/ch28.tex`
- Original label: `tab:ch28-destruction-fixes`
- Original caption (verbatim):

> SSA-destruction hazards with their standard repairs.

| Hazard | Symptom | Repair |
| --- | --- | --- |
| Critical edge | No single home for copy | Split the edge |
| Lost copy | Overwritten shared location | Parallel copy plus split |
| Swap | Crossed sources clobbered | Temporary in sequence |
| Dead phi | Useless copies survive | Dead-code pass after |

<!-- Original LaTeX -->

```latex
\begin{tabular}{lll}
\toprule
Hazard & Symptom & Repair \\
\midrule
Critical edge & No single home for copy & Split the edge \\
Lost copy & Overwritten shared location & Parallel copy plus split \\
Swap & Crossed sources clobbered & Temporary in sequence \\
Dead phi & Useless copies survive & Dead-code pass after \\
\bottomrule
\end{tabular}
```
