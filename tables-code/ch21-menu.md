# Memory optimizations with the fact that makes each one legal.

- Source file: `chapters/ch21.tex`
- Original label: `tab:ch21-menu`
- Original caption (verbatim):

> Memory optimizations with the fact that makes each one legal.

| Optimization | What it removes | Legality fact |
| --- | --- | --- |
| Load elimination | Repeated loads | Address available, no may-store |
| Store forwarding | Load after store | Must-alias with nothing between |
| Dead stores | Unread stores | Location not live on any path |
| Scalar replacement | Loop traffic | All observers accounted for |
| Invariant loads | Repeated hoists | Address invariant, safe to speculate |
| Layout tuning | Wasted lines | Unobservable layout, updated uses |

<!-- Original LaTeX -->

```latex
\begin{tabular}{lll}
    \toprule
    Optimization & What it removes & Legality fact \\
    \midrule
    Load elimination & Repeated loads & Address available, no may-store \\
    Store forwarding & Load after store & Must-alias with nothing between \\
    Dead stores & Unread stores & Location not live on any path \\
    Scalar replacement & Loop traffic & All observers accounted for \\
    Invariant loads & Repeated hoists & Address invariant, safe to speculate \\
    Layout tuning & Wasted lines & Unobservable layout, updated uses \\
    \bottomrule
  \end{tabular}
```
