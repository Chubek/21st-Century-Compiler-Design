# Tiers trade compile speed against code quality, with profiling connecting each step upward.

- Source file: `chapters/ch34.tex`
- Original label: `tab:ch34-tiers`
- Original caption (verbatim):

> Tiers trade compile speed against code quality, with profiling connecting each step upward.

| Tier | Compile speed | Code quality | Needs |
| --- | --- | --- | --- |
| Interpreter | None | Slowest | Counters |
| Baseline | Fast | Modest | Hot threshold |
| Optimizing | Slow | Best | Type profiles |

<!-- Original LaTeX -->

```latex
\begin{tabular}{llll}
\toprule
Tier & Compile speed & Code quality & Needs \\
\midrule
Interpreter & None & Slowest & Counters \\
Baseline & Fast & Modest & Hot threshold \\
Optimizing & Slow & Best & Type profiles \\
\bottomrule
\end{tabular}
```
