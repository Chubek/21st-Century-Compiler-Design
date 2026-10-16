# Benefits of inlining and specialization with their main risks.

- Source file: `chapters/ch23.tex`
- Original label: `tab:ch23-inline-tradeoffs`
- Original caption (verbatim):

> Benefits of inlining and specialization with their main risks.

| Benefit | Mechanism | Risk |
| --- | --- | --- |
| Eliminated call overhead | No jump or frame setup | Larger code size |
| Exposed constants | Arguments become local facts | Extra register pressure |
| Better branch folding | Callee tests meet caller values | More hot-path branches |
| Type refinement | Clone assumes one case | Guard cost on mismatch |
| Cross-call code motion | One region for hoisting | Slower compile time |

<!-- Original LaTeX -->

```latex
\begin{tabular}{lll}
\toprule
Benefit & Mechanism & Risk \\
\midrule
Eliminated call overhead & No jump or frame setup & Larger code size \\
Exposed constants & Arguments become local facts & Extra register pressure \\
Better branch folding & Callee tests meet caller values & More hot-path branches \\
Type refinement & Clone assumes one case & Guard cost on mismatch \\
Cross-call code motion & One region for hoisting & Slower compile time \\
\bottomrule
\end{tabular}
```
