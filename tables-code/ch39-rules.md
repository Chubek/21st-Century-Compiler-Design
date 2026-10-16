# Typing rules for Mini-Fun. Each rule is selected by the shape of e; the context Gamma maps in-scope variables to types.

- Source file: `chapters/ch39.tex`
- Original label: `tab:ch39-rules`
- Original caption (verbatim):

> Typing rules for Mini-Fun. Each rule is selected by the shape of $e$; the context $\Gamma$ maps in-scope variables to types.

| Rule | Conclusion from premises |
| --- | --- |
| Var | `Gamma |- x : Gamma(x)` when `x` is bound in `Gamma` |
| Int | `Gamma |- n : int` for every integer literal `n` |
| Bool | `Gamma |- b : bool` for `b` true or false |
| Add | `Gamma |- e1 + e2 : int` when each side has type `int` |
| Cmp | `Gamma |- e1 < e2 : bool` when each side has type `int` |
| If | `Gamma |- if e then e1 else e2 : T` when `e : bool` and both arms have type `T` |
| Lam | `Gamma |- lam(x : T1, e) : T1 -> T2` when `Gamma, x : T1 |- e : T2` |
| App | `Gamma |- app(e1, e2) : T2` when `e1 : T1 -> T2` and `e2 : T1` |

<!-- Original LaTeX -->

```latex
\begin{tabular}{ll}
\toprule
Rule & Conclusion from premises \\
\midrule
Var & $\Gamma \vdash x : \Gamma(x)$ when $x$ is bound in $\Gamma$ \\
Int & $\Gamma \vdash n : \operatorname{int}$ for every integer literal $n$ \\
Bool & $\Gamma \vdash b : \operatorname{bool}$ for $b$ true or false \\
Add & $\Gamma \vdash e_1 + e_2 : \operatorname{int}$ when each side has type $\operatorname{int}$ \\
Cmp & $\Gamma \vdash e_1 < e_2 : \operatorname{bool}$ when each side has type $\operatorname{int}$ \\
If & $\Gamma \vdash \operatorname{if}\,e\,\operatorname{then}\,e_1\,\operatorname{else}\,e_2 : T$ when $e : \operatorname{bool}$ and both arms have type $T$ \\
Lam & $\Gamma \vdash \operatorname{lam}(x : T_1, e) : T_1 \to T_2$ when $\Gamma, x : T_1 \vdash e : T_2$ \\
App & $\Gamma \vdash \operatorname{app}(e_1, e_2) : T_2$ when $e_1 : T_1 \to T_2$ and $e_2 : T_1$ \\
\bottomrule
\end{tabular}
```
