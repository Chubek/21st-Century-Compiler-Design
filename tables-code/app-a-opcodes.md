# Each opcode has a fixed stack effect.

- Source file: `appendices/appendix-a.tex`
- Original label: `tab:app-a-opcodes`
- Original caption (verbatim):

> Each opcode has a fixed stack effect.

| Opcode | Operands removed | Value pushed |
| --- | --- | --- |
| `PUSH` n | none | n, with n in V |
| `GET_INPUT` | none | current input a |
| `ADD8` | u,v | (u+v) mod 256 |
| `MUL8` | u,v | (u × v) mod 256 |

<!-- Original LaTeX -->

```latex
\begin{tabular}{lll}
    \toprule
    Opcode & Operands removed & Value pushed \\
    \midrule
    \code{PUSH} $n$ & none & $n$, with $n\in V$ \\
    \code{GET\_INPUT} & none & current input $a$ \\
    \code{ADD8} & $u,v$ & $(u+v)\bmod256$ \\
    \code{MUL8} & $u,v$ & $(u\times v)\bmod256$ \\
    \bottomrule
  \end{tabular}
```
