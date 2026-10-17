# Logic chapter checks

`checks.c` independently enumerates the finite examples in
`chapters/ch40.tex` and `chapters/ch41.tex`. It requires a C11 compiler and
the standard library. Keep assertions enabled.

```sh
cc -std=c11 -Wall -Wextra -Wpedantic -Werror \
  example-code/logic-checks/checks.c -o /tmp/opencode/logic-checks
/tmp/opencode/logic-checks
```

Coverage:

- All four rows of the propositional table and the entailment exercise.
- Quantifier order in the two-use/two-definition relation and the finite
  capture-avoidance example.
- Assignment and branch preconditions on initial integers from -32 to 32.
- Loop invariants and variants for bounds from 0 to 64, including both
  increments used by the chapter and its exercises.
- Every assignment for the NOT, AND, and OR clause templates.
- Every assignment to the six variables of the worked Tseitin encoding;
  root assertion and unique extension are checked separately.
- The unit-propagation example and contradictory-root exercise.
- All four-bit addition boundary cases and the doubling/halving rule at
  every width from 1 through 8.

The checker models small-width arithmetic using ordinary `unsigned` values
and explicit masks. It does not depend on narrow C integer promotions or
signed overflow. Its finite checks do not prove the unbounded integer
statements; `SOLUTIONS.md` supplies the algebraic arguments for the exercises.
No external SAT or SMT solver is required or invoked.
