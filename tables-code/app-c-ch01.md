# Set, evaluation, translation, and behavior symbols used in the opening chapter.

- Source file: `appendices/appendix-c.tex`
- Original label: `tab:app-c-ch01`
- Original caption (verbatim):

> Set, evaluation, translation, and behavior symbols used in the opening chapter.

| Symbol | Meaning |
| --- | --- |
| `\set{x \| P(x)}` | set of values x satisfying property P |
| V | tiny-language value set {0,1,...,255} |
| [[e]]_a | value of expression e at input a |
| F(e) | constant-folded form of expression e |
| C(e) | stack instruction sequence compiled from e |
| Exec(c,a,S) | result of running code c at input a from stack S |
| S‖ [v] | stack S extended with value v |
| D(e) | peak extra stack slots needed by C(e) |
| (t,r) | terminating execution with trace t and result r |
| B(P,i) | set of behaviors permitted for program P on input i |
| run(C(P),i) | observed execution of compiled program on input i |

<!-- Original LaTeX -->

```latex
\begin{tabular}{ll}
    \toprule
    Symbol & Meaning \\
    \midrule
    \texttt{\textbackslash set\{x | P(x)\}} & set of values $x$ satisfying property $P$ \\
    $V$ & tiny-language value set $\{0,1,\ldots,255\}$ \\
    $\llbracket e\rrbracket_a$ & value of expression $e$ at input $a$ \\
    $F(e)$ & constant-folded form of expression $e$ \\
    $C(e)$ & stack instruction sequence compiled from $e$ \\
    $\operatorname{Exec}(c,a,S)$ & result of running code $c$ at input $a$ from stack $S$ \\
    $S\mathbin{\|} [v]$ & stack $S$ extended with value $v$ \\
    $D(e)$ & peak extra stack slots needed by $C(e)$ \\
    $(t,r)$ & terminating execution with trace $t$ and result $r$ \\
    $\mathcal{B}(P,i)$ & set of behaviors permitted for program $P$ on input $i$ \\
    $\operatorname{run}(C(P),i)$ & observed execution of compiled program on input $i$ \\
    \bottomrule
  \end{tabular}
```
