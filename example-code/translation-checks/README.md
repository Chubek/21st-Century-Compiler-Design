# Translation-validation examples (C11)

`checks.c` backs the chapter **Verifying Compiler Transformations**, source
`chapters/ch47.tex`. It checks exact deterministic outcomes in a byte-valued IR
with wrapping arithmetic and a defined zero-division trap. It is a bounded
example validator, not an SMT solver or a general compiler equivalence checker.

From the repository root, keep assertions enabled:

```sh
cc -std=c11 -Wall -Wextra -Wpedantic -Werror \
  example-code/translation-checks/checks.c -o /tmp/opencode/translation-checks
/tmp/opencode/translation-checks
```

The program exhausts 256 inputs for each scalar rewrite and 512 input pairs for
the conditional division. It also checks paired loop boundaries for all bounds
0 through 64 against scalar execution and a separate sum loop. None of these
loop tests establishes an unbounded theorem; the chapter gives that argument
under mathematical-integer semantics. All host intermediates in the tests fit
the minimum C11 unsigned range. The division-by-zero outcome is represented
explicitly and never executed as C undefined behavior.

Repeat with `gcc` and `clang`, adding `-fsanitize=address,undefined` for an
instrumented run. There are no third-party dependencies.
