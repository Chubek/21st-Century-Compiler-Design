# Diagnostic kinds in Mini with a triggering example and the message each one produces.

- Source file: `chapters/ch05.tex`
- Original label: `tab:ch05-diagnostics`
- Original caption (verbatim):

> Diagnostic kinds in Mini with a triggering example and the message each one produces.

| Kind | Triggering example | Message shape |
| --- | --- | --- |
| Undefined variable | `print y` | undefined name `y` at line L |
| Redefinition | `let x = 1 in let x = 2` | allowed shadowing, no error |
| Type mismatch | `1 + (2 > 3)` | operator needs integers, found boolean |
| Bad condition | `if 3 then print 1` | condition must be boolean, found integer |
| Arity error | `print` with no operand | `print` needs one operand |

<!-- Original LaTeX -->

```latex
\begin{tabular}{lll}
\toprule
Kind & Triggering example & Message shape \\
\midrule
Undefined variable & \code{print y} & undefined name \code{y} at line $L$ \\
Redefinition & \code{let x = 1 in let x = 2} & allowed shadowing, no error \\
Type mismatch & \code{1 + (2 > 3)} & operator needs integers, found boolean \\
Bad condition & \code{if 3 then print 1} & condition must be boolean, found integer \\
Arity error & \code{print} with no operand & \code{print} needs one operand \\
\bottomrule
\end{tabular}
```
