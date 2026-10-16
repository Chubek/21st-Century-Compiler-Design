# Common selection patterns and the IR they absorb.

- Source file: `chapters/ch25.tex`
- Original label: `tab:ch25-pattern-examples`
- Original caption (verbatim):

> Common selection patterns and the IR they absorb.

| Pattern | IR fragment absorbed | Benefit |
| --- | --- | --- |
| Load with displacement | `add` plus `load` | Fewer instructions |
| Scaled indexed load | `mul`, `add`, `load` | No separate multiply |
| Multiply-accumulate | `mul` plus `add` | One fused operation |
| Compare and branch | `cmp` plus `br` | Uses flag-setting form |
| Move immediate | Constant plus `mov` | Shorter encoding |

<!-- Original LaTeX -->

```latex
\begin{tabular}{lll}
\toprule
Pattern & IR fragment absorbed & Benefit \\
\midrule
Load with displacement & \code{add} plus \code{load} & Fewer instructions \\
Scaled indexed load & \code{mul}, \code{add}, \code{load} & No separate multiply \\
Multiply-accumulate & \code{mul} plus \code{add} & One fused operation \\
Compare and branch & \code{cmp} plus \code{br} & Uses flag-setting form \\
Move immediate & Constant plus \code{mov} & Shorter encoding \\
\bottomrule
\end{tabular}
```
