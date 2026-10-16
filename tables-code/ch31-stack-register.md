# Stack and register designs differ in encoding density and dispatch count rather than expressiveness.

- Source file: `chapters/ch31.tex`
- Original label: `tab:ch31-stack-register`
- Original caption (verbatim):

> Stack and register designs differ in encoding density and dispatch count rather than expressiveness.

| Property | Stack machine | Register machine | Consequence |
| --- | --- | --- | --- |
| Operands | Implicit | Explicit | Stack code is shorter |
| Dispatches | More | Fewer | Registers save steps |
| Verification | Simpler | Richer | Stack types track depth |
| Mapping | Indirect | Direct | Registers map to hardware |

<!-- Original LaTeX -->

```latex
\begin{tabular}{llll}
\toprule
Property & Stack machine & Register machine & Consequence \\
\midrule
Operands & Implicit & Explicit & Stack code is shorter \\
Dispatches & More & Fewer & Registers save steps \\
Verification & Simpler & Richer & Stack types track depth \\
Mapping & Indirect & Direct & Registers map to hardware \\
\bottomrule
\end{tabular}
```
