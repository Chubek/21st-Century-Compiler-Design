# Calling-convention decisions and what each choice costs.

- Source file: `chapters/ch24.tex`
- Original label: `tab:ch24-convention-choices`
- Original caption (verbatim):

> Calling-convention decisions and what each choice costs.

| Decision | Register-heavy choice | Stack-heavy choice |
| --- | --- | --- |
| Argument passing | More registers, less memory traffic | Simpler ABI, more loads |
| Callee-saved set | Fewer caller spills | Larger prologues |
| Return by value | Registers for small types | Hidden pointer, extra copy |
| Variadic calls | Fixed register sequence | Stack overflow area |
| Alignment | 16-byte boundary always | Smaller but fragile frames |

<!-- Original LaTeX -->

```latex
\begin{tabular}{lll}
\toprule
Decision & Register-heavy choice & Stack-heavy choice \\
\midrule
Argument passing & More registers, less memory traffic & Simpler ABI, more loads \\
Callee-saved set & Fewer caller spills & Larger prologues \\
Return by value & Registers for small types & Hidden pointer, extra copy \\
Variadic calls & Fixed register sequence & Stack overflow area \\
Alignment & 16-byte boundary always & Smaller but fragile frames \\
\bottomrule
\end{tabular}
```
