# Analysis chapter checks

`checks.c` independently recomputes the examples and exercise arithmetic in
`chapters/ch42.tex`, `chapters/ch43.tex`, and `chapters/ch44.tex`.
It uses C11 and the standard library only. Keep assertions enabled.

From the repository root:

```sh
cc -std=c11 -Wall -Wextra -Wpedantic -Werror -O2 \
  example-code/analysis-checks/checks.c -o /tmp/opencode/analysis-checks
/tmp/opencode/analysis-checks
```

The checks enumerate concrete states independently of the abstract operations:

- branch value sets, interval-square loss, wraparound, and certificate failure;
- closed DBM entries versus maxima over concrete pairs, guard inconsistency,
  widening/replay versus complete loop traces, and path-merge loss;
- synchronous recursive summaries versus direct recursive execution, matched
  returns, and concrete heap stores versus strong/weak abstract updates.

The small DBM implementation is a checking fixture, not a production domain.
Its finite infinity sentinel is sufficient for these explicitly bounded examples;
production implementations need the exact or conservatively bounded arithmetic
described in the text. The arithmetic examples model mathematical integers unless
the check explicitly applies modulo 256. No C signed overflow is used.

`SOLUTIONS.md` records answers to every exercise in the three chapters.
These checks do not compile the book; the integrating agent owns that build.
