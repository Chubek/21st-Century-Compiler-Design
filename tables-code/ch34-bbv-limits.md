# Version-budget choices with their payoff and danger.

- Source file: `chapters/ch34.tex`
- Original label: `tab:ch34-bbv-limits`
- Original caption (verbatim):

> Version-budget choices with their payoff and danger.

| Choice | Payoff | Danger |
| --- | --- | --- |
| Small budget (1--2) | little code growth | misses rarer contexts |
| Large budget | covers more contexts | code growth and compile time |
| Lazy only | pays only for seen behavior | first new context diverts |
| Generic fallback kept | correctness without full coverage | fallback stays slow |

<!-- Original LaTeX -->

```latex
\begin{tabular}{lll}
\toprule
Choice & Payoff & Danger \\
\midrule
Small budget ($1$--$2$) & little code growth & misses rarer contexts \\
Large budget & covers more contexts & code growth and compile time \\
Lazy only & pays only for seen behavior & first new context diverts \\
Generic fallback kept & correctness without full coverage & fallback stays slow \\
\bottomrule
\end{tabular}
```
