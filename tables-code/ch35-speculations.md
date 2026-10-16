# Each speculation pairs an assumed fact with a guard and a tested fallback.

- Source file: `chapters/ch35.tex`
- Original label: `tab:ch35-speculations`
- Original caption (verbatim):

> Each speculation pairs an assumed fact with a guard and a tested fallback.

| Assumption | Guard | Fallback |
| --- | --- | --- |
| Operand is integer | Tag test | Generic arithmetic |
| Branch goes one way | Direction test | Cold path |
| Call hits one target | Class test | Full dispatch |
| Object has one shape | Shape test | Generic access |

<!-- Original LaTeX -->

```latex
\begin{tabular}{lll}
\toprule
Assumption & Guard & Fallback \\
\midrule
Operand is integer & Tag test & Generic arithmetic \\
Branch goes one way & Direction test & Cold path \\
Call hits one target & Class test & Full dispatch \\
Object has one shape & Shape test & Generic access \\
\bottomrule
\end{tabular}
```
