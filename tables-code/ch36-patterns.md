# Loop patterns differ in dependence shape and in their threading verdict.

- Source file: `chapters/ch36.tex`
- Original label: `tab:ch36-patterns`
- Original caption (verbatim):

> Loop patterns differ in dependence shape and in their threading verdict.

| Pattern | Dependence | Verdict |
| --- | --- | --- |
| Element-wise map | None | Vectorize and parallelize |
| Reduction | Combine only | Tree reduce then combine |
| Stencil, shift 1 | Bounded neighbors | Vectorize with alignment plan |
| Recurrence | Loop-carried chain | Reject without rewrite |

<!-- Original LaTeX -->

```latex
\begin{tabular}{lll}
\toprule
Pattern & Dependence & Verdict \\
\midrule
Element-wise map & None & Vectorize and parallelize \\
Reduction & Combine only & Tree reduce then combine \\
Stencil, shift 1 & Bounded neighbors & Vectorize with alignment plan \\
Recurrence & Loop-carried chain & Reject without rewrite \\
\bottomrule
\end{tabular}
```
