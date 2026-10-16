# Alias answers derived from the fixed point by intersecting target sets.

- Source file: `chapters/ch20.tex`
- Original label: `tab:ch20-queries`
- Original caption (verbatim):

> Alias answers derived from the fixed point by intersecting target sets.

| Query pair | Shared targets | Answer |
| --- | --- | --- |
| `*p` versus `*q` | y | may-alias |
| `*r` versus `x` | x | may-alias |
| `*q` versus `*s` | none | no-alias |
| `*p` versus `*s` | x | may-alias |

<!-- Original LaTeX -->

```latex
\begin{tabular}{lll}
    \toprule
    Query pair & Shared targets & Answer \\
    \midrule
    \code{*p} versus \code{*q} & $y$ & may-alias \\
    \code{*r} versus \code{x} & $x$ & may-alias \\
    \code{*q} versus \code{*s} & none & no-alias \\
    \code{*p} versus \code{*s} & $x$ & may-alias \\
    \bottomrule
  \end{tabular}
```
