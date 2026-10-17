# Aurocks examples

`backtrack.g` tests reopening a locally successful callee. Inputs `ab` and `abb`
produce status 0 and two actions; `abx` produces syntax status 1 and zero actions.
`calculator.g` demonstrates sums, products, parentheses, nullable tails, and
result-owned semantic values. Its driver caps input at nine one-digit leaves
to keep all arithmetic within the minimum C99 `long` range.

The executable generator and template live in `support/`, as requested for the
book's Perl-to-C99 case study. See [`support/aurocks.md`](../../support/aurocks.md)
for commands, language/API contracts, and the end-to-end test suite. Generated
C and binaries belong outside the repository, normally in `/tmp/opencode`.

`SOLUTIONS.md` contains checked answers for all four front-end expansion chapters.
