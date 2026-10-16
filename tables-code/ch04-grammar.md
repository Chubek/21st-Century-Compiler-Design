# The Mini grammar with one rule per syntactic category and grouping fixed by rule layering.

- Source file: `chapters/ch04.tex`
- Original label: `tab:ch04-grammar`
- Original caption (verbatim):

> The Mini grammar with one rule per syntactic category and grouping fixed by rule layering.

| Category | Forms |
| --- | --- |
| Program | statement followed by more statements |
| Statement | assignment, let binding, print, conditional, loop, or block |
| Assignment | identifier, `=`, expression |
| Expression | product, then any number of add or subtract steps |
| Product | atom, then any number of multiply or divide steps |
| Atom | number, variable, or parenthesized expression |
| Conditional | `if` test `then` branch `else` branch |
| Loop | `while` test `do` body |

<!-- Original LaTeX -->

```latex
\begin{tabular}{ll}
\toprule
Category & Forms \\
\midrule
Program & statement followed by more statements \\
Statement & assignment, let binding, print, conditional, loop, or block \\
Assignment & identifier, \code{=}, expression \\
Expression & product, then any number of add or subtract steps \\
Product & atom, then any number of multiply or divide steps \\
Atom & number, variable, or parenthesized expression \\
Conditional & \code{if} test \code{then} branch \code{else} branch \\
Loop & \code{while} test \code{do} body \\
\bottomrule
\end{tabular}
```
