# Tagging schemes trade value capacity against test cost and collector complexity.

- Source file: `chapters/ch32.tex`
- Original label: `tab:ch32-tagging`
- Original caption (verbatim):

> Tagging schemes trade value capacity against test cost and collector complexity.

| Scheme | Capacity | Test cost | Notes |
| --- | --- | --- | --- |
| Low-bit tags | 62-bit integer | Mask and compare | Needs aligned pointers |
| NaN boxing | 51-bit payload | Range compare | Shares float encoding |
| Kind word | Full width | One load | Wider values everywhere |
| Static proof | Full width | Guard on change | Needs fallback path |

<!-- Original LaTeX -->

```latex
\begin{tabular}{llll}
\toprule
Scheme & Capacity & Test cost & Notes \\
\midrule
Low-bit tags & 62-bit integer & Mask and compare & Needs aligned pointers \\
NaN boxing & 51-bit payload & Range compare & Shares float encoding \\
Kind word & Full width & One load & Wider values everywhere \\
Static proof & Full width & Guard on change & Needs fallback path \\
\bottomrule
\end{tabular}
```
