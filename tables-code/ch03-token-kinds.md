# Token kinds of Mini with example lexemes and recognition rules.

- Source file: `chapters/ch03.tex`
- Original label: `tab:ch03-token-kinds`
- Original caption (verbatim):

> Token kinds of Mini with example lexemes and recognition rules.

| Kind | Example lexemes | Recognition rule |
| --- | --- | --- |
| Identifier | `x`, `total2` | letter followed by letters or digits |
| Number | `10`, `007` | one or more digits |
| Operator | `+`, `<=`, `=` | longest match from the operator set |
| Keyword | `if`, `while`, `print` | exact reserved spelling |
| Punctuation | `;`, `(`, `)` | single fixed character |
| End of input | none | position past the last character |

<!-- Original LaTeX -->

```latex
\begin{tabular}{lll}
\toprule
Kind & Example lexemes & Recognition rule \\
\midrule
Identifier & \code{x}, \code{total2} & letter followed by letters or digits \\
Number & \code{10}, \code{007} & one or more digits \\
Operator & \code{+}, \code{<=}, \code{=} & longest match from the operator set \\
Keyword & \code{if}, \code{while}, \code{print} & exact reserved spelling \\
Punctuation & \code{;}, \code{(}, \code{)} & single fixed character \\
End of input & none & position past the last character \\
\bottomrule
\end{tabular}
```
