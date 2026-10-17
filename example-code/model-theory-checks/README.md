# Transition-system arithmetic checks

These C11 checks recompute the finite examples and exercise answers in
`chapters/ch45.tex` and `chapters/ch46.tex`. They require only a C11 compiler
and the standard library. Keep assertions enabled.

```sh
cc -std=c11 -Wall -Wextra -Wpedantic -Werror checks.c -o /tmp/opencode/model-theory-checks
/tmp/opencode/model-theory-checks
```

The small bit-set routines are reference calculations, not the companion
checker in Chapter 48. The checks compare fixed-point results with bounded
path enumeration for every operand set on the six-state job graph. They also
check the displayed iteration sequences, BFS distances, monitor histories,
fairness examples, and concrete-prefix validation.

`SOLUTIONS.md` records the reasoning behind all eleven exercises.
