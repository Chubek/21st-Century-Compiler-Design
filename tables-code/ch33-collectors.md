# Collector designs uphold the same safety rule but pay at different times and fragment differently.

- Source file: `chapters/ch33.tex`
- Original label: `tab:ch33-collectors`
- Original caption (verbatim):

> Collector designs uphold the same safety rule but pay at different times and fragment differently.

| Design | Pause shape | Steady cost | Fragmentation |
| --- | --- | --- | --- |
| Mark-sweep | Mark pause | Mark per live object | Free lists fragment |
| Semi-space | Copy pause | Copy per survivor | Survivors stay compact |
| Ref. counting | Small steps | Count per update | Needs cycle handling |

<!-- Original LaTeX -->

```latex
\begin{tabular}{llll}
\toprule
Design & Pause shape & Steady cost & Fragmentation \\
\midrule
Mark-sweep & Mark pause & Mark per live object & Free lists fragment \\
Semi-space & Copy pause & Copy per survivor & Survivors stay compact \\
Ref. counting & Small steps & Count per update & Needs cycle handling \\
\bottomrule
\end{tabular}
```
