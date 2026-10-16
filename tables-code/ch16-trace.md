# Value numbers assigned while scanning Listing from top to bottom.

- Source file: `chapters/ch16.tex`
- Original label: `tab:ch16-trace`
- Original caption (verbatim):

> Value numbers assigned while scanning Listing~\ref{lst:ch16-block} from top to bottom.

| Line | Instruction | Key | Action |
| --- | --- | --- | --- |
| 1 | `t1 = x + y` | plus(1,2) | new number 3 |
| 2 | `t2 = y + x` | plus(1,2) | reuse 3, copy `t1` |
| 3 | `t3 = 2 + 3` | const 5 | new number 4 |
| 4 | `t4 = t1 * t3` | mul(3,4) | new number 5 |
| 5 | `x = 0` | const 0 | new number 6, update holder |
| 6 | `t5 = x + y` | plus(6,2) | new number 7 |
| 7 | `t6 = t1 * t3` | mul(3,4) | reuse 5, copy `t4` |

<!-- Original LaTeX -->

```latex
\begin{tabular}{llll}
    \toprule
    Line & Instruction & Key & Action \\
    \midrule
    1 & \code{t1 = x + y} & $plus(1,2)$ & new number 3 \\
    2 & \code{t2 = y + x} & $plus(1,2)$ & reuse 3, copy \code{t1} \\
    3 & \code{t3 = 2 + 3} & const 5 & new number 4 \\
    4 & \code{t4 = t1 * t3} & $mul(3,4)$ & new number 5 \\
    5 & \code{x = 0} & const 0 & new number 6, update holder \\
    6 & \code{t5 = x + y} & $plus(6,2)$ & new number 7 \\
    7 & \code{t6 = t1 * t3} & $mul(3,4)$ & reuse 5, copy \code{t4} \\
    \bottomrule
  \end{tabular}
```
