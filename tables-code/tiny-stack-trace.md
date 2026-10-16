# The seven-instruction program produces 8 at input 250.

- Source file: `chapters/ch01/stack-translation.tex`
- Original label: `tab:tiny-stack-trace`
- Original caption (verbatim):

> The seven-instruction program produces 8 at input 250.

| Position | Instruction | Stack after execution |
| --- | --- | --- |
|  | Before execution | [] |
| 0 | `GET_INPUT` | [250] |
| 1 | `PUSH` 7 | [250,7] |
| 2 | `ADD8` | [1] |
| 3 | `PUSH` 3 | [1,3] |
| 4 | `PUSH` 5 | [1,3,5] |
| 5 | `ADD8` | [1,8] |
| 6 | `MUL8` | [8] |

<!-- Original LaTeX -->

```latex
\begin{tabular}{rll}
    \toprule
    Position & Instruction & Stack after execution \\
    \midrule
    & Before execution & $[\,]$ \\
    0 & \code{GET\_INPUT} & $[250]$ \\
    1 & \code{PUSH} 7 & $[250,7]$ \\
    2 & \code{ADD8} & $[1]$ \\
    3 & \code{PUSH} 3 & $[1,3]$ \\
    4 & \code{PUSH} 5 & $[1,3,5]$ \\
    5 & \code{ADD8} & $[1,8]$ \\
    6 & \code{MUL8} & $[8]$ \\
    \bottomrule
  \end{tabular}
```
