# Checked exercise answers: Verifying Compiler Transformations

- **Byte guard:** inputs 0 through 254, exactly the interval `[0,254]`, are valid.
  At 255 the source comparison is `0 > 255`, false, and the candidate is true.
- **Event hoist:** choose condition 0 and divisor 3. The source returns zero with
  no event; the target emits the event, computes 6, and returns zero. A result-only
  comparator misses the extra event. Requiring a nonzero divisor fixes the trap
  example but does not justify this effectful hoist.
- **Paired loops, n=6:** `(0,0)`, `(2,1)`, `(4,6)`, `(6,15)`. The cleanup guard
  `j<n` is false. A ranking argument needs a nonnegative mathematical quantity
  that strictly decreases; wrapping subtraction can jump to a large value after
  underflow and invalidates that argument without suitable guard/range premises.
- **True certificate:** initial states imply true and each transition preserves
  true. Thus all initialization/consecution obligations can hold. The client
  obligation `true => divisor != 0` is false, so division-check elimination is
  not established.

The finite results are recomputed by `checks.c`; the loop argument and effect
counterexample are additionally worked in the chapter.
