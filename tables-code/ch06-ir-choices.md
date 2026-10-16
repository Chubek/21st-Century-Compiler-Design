# Intermediate-form choices with what each one makes explicit and what it costs.

- Source file: `chapters/ch06.tex`
- Original label: `tab:ch06-ir-choices`
- Original caption (verbatim):

> Intermediate-form choices with what each one makes explicit and what it costs.

| Choice | Makes explicit | Main cost |
| --- | --- | --- |
| Tree plus symbols | source nesting and names | analyses dig through nesting |
| Flat three-address list | order, temporaries, branches | control joins need a graph |
| Control-flow graph | blocks, edges, joins | construction and repair after edits |
| SSA discipline | one definition per name | merge operations and destruction |
| Typed temporaries | kinds checkable per pass | annotation and rechecking time |

<!-- Original LaTeX -->

```latex
\begin{tabular}{lll}
\toprule
Choice & Makes explicit & Main cost \\
\midrule
Tree plus symbols & source nesting and names & analyses dig through nesting \\
Flat three-address list & order, temporaries, branches & control joins need a graph \\
Control-flow graph & blocks, edges, joins & construction and repair after edits \\
SSA discipline & one definition per name & merge operations and destruction \\
Typed temporaries & kinds checkable per pass & annotation and rechecking time \\
\bottomrule
\end{tabular}
```
