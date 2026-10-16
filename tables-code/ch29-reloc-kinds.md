# Common relocation computations and when they apply.

- Source file: `chapters/ch29.tex`
- Original label: `tab:ch29-reloc-kinds`
- Original caption (verbatim):

> Common relocation computations and when they apply.

| Computation | Meaning | Typical use |
| --- | --- | --- |
| Direct 32 or 64 | Symbol address plus addend | Absolute pointers |
| PC-relative 32 | Symbol minus place plus addend | Calls and jumps |
| GOT-relative | Table offset of symbol | Shared-library data |
| PLT call | Stub address for function | Lazy binding |
| TLS variant | Thread-block offset | Thread-local data |

<!-- Original LaTeX -->

```latex
\begin{tabular}{lll}
\toprule
Computation & Meaning & Typical use \\
\midrule
Direct 32 or 64 & Symbol address plus addend & Absolute pointers \\
PC-relative 32 & Symbol minus place plus addend & Calls and jumps \\
GOT-relative & Table offset of symbol & Shared-library data \\
PLT call & Stub address for function & Lazy binding \\
TLS variant & Thread-block offset & Thread-local data \\
\bottomrule
\end{tabular}
```
