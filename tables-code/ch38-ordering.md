# Ordering rules pair each sequence constraint with the defect it prevents.

- Source file: `chapters/ch38.tex`
- Original label: `tab:ch38-ordering`
- Original caption (verbatim):

> Ordering rules pair each sequence constraint with the defect it prevents.

| Rule | Order | Prevents |
| --- | --- | --- |
| Analyze then change | Facts before rewrites | Stale-fact miscompilation |
| Clean then measure | Cleanup before timing | Dead-code timing noise |
| Lower late | Machine detail last | Blocked portable passes |

<!-- Original LaTeX -->

```latex
\begin{tabular}{lll}
\toprule
Rule & Order & Prevents \\
\midrule
Analyze then change & Facts before rewrites & Stale-fact miscompilation \\
Clean then measure & Cleanup before timing & Dead-code timing noise \\
Lower late & Machine detail last & Blocked portable passes \\
\bottomrule
\end{tabular}
```
