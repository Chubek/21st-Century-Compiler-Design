# Three-address instructions used by Mini lowering with the effect of each opcode.

- Source file: `chapters/ch06.tex`
- Original label: `tab:ch06-opcodes`
- Original caption (verbatim):

> Three-address instructions used by Mini lowering with the effect of each opcode.

| Instruction | Effect |
| --- | --- |
| `t = a op b` | applies one arithmetic or comparison to named values |
| `t = a` | copies one named value or constant |
| `t = const` | materializes a literal integer |
| `label L` | marks a position that branches can target |
| `br c, L1, L2` | continues at `L1` if `c` is true, else `L2` |
| `jmp L` | continues unconditionally at `L` |
| `print a` | appends the named value to the output |

<!-- Original LaTeX -->

```latex
\begin{tabular}{ll}
\toprule
Instruction & Effect \\
\midrule
\code{t = a op b} & applies one arithmetic or comparison to named values \\
\code{t = a} & copies one named value or constant \\
\code{t = const} & materializes a literal integer \\
\code{label L} & marks a position that branches can target \\
\code{br c, L1, L2} & continues at \code{L1} if \code{c} is true, else \code{L2} \\
\code{jmp L} & continues unconditionally at \code{L} \\
\code{print a} & appends the named value to the output \\
\bottomrule
\end{tabular}
```
