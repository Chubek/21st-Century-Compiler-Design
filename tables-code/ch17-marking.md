# Invariant marking over Listing reaches a fixed point after two passes with one member.

- Source file: `chapters/ch17.tex`
- Original label: `tab:ch17-marking`
- Original caption (verbatim):

> Invariant marking over Listing~\ref{lst:ch17-loop} reaches a fixed point after two passes with one member.

| Candidate | Pass 1 | Pass 2 |
| --- | --- | --- |
| `t = n * 4` | invariant | invariant |
| `u = i * 4` | variant | variant |
| `v = t + u` | variant | variant |
| `snext = s + v` | variant | variant |
| `inext = i + 1` | variant | variant |

<!-- Original LaTeX -->

```latex
\begin{tabular}{lll}
    \toprule
    Candidate & Pass 1 & Pass 2 \\
    \midrule
    \code{t = n * 4} & invariant & invariant \\
    \code{u = i * 4} & variant & variant \\
    \code{v = t + u} & variant & variant \\
    \code{snext = s + v} & variant & variant \\
    \code{inext = i + 1} & variant & variant \\
    \bottomrule
  \end{tabular}
```
