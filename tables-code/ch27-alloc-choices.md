# Allocator decisions with their payoff and danger.

- Source file: `chapters/ch27.tex`
- Original label: `tab:ch27-alloc-choices`
- Original caption (verbatim):

> Allocator decisions with their payoff and danger.

| Decision | Payoff | Danger |
| --- | --- | --- |
| Spill cold range | Small traffic cost | Wrong range chosen |
| Split hot region | Traffic only at edges | Extra boundary moves |
| Coalesce moves | Fewer copies | Uncolorable merged node |
| Reserve registers | Simpler calls | Higher base pressure |
| Rematerialize | No memory traffic | Recomputation cost |

<!-- Original LaTeX -->

```latex
\begin{tabular}{lll}
\toprule
Decision & Payoff & Danger \\
\midrule
Spill cold range & Small traffic cost & Wrong range chosen \\
Split hot region & Traffic only at edges & Extra boundary moves \\
Coalesce moves & Fewer copies & Uncolorable merged node \\
Reserve registers & Simpler calls & Higher base pressure \\
Rematerialize & No memory traffic & Recomputation cost \\
\bottomrule
\end{tabular}
```
