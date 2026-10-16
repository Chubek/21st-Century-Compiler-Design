# Chapter 1 examples

Requires a C11 compiler and its standard library. No external C dependencies.
Run from this directory; put executables outside the manuscript tree:

```sh
cc -std=c11 -Wall -Wextra -Wpedantic -Werror expr.c test_expr.c -o /tmp/opencode/ch01-test
/tmp/opencode/ch01-test
cc -std=c11 -Wall -Wextra -Wpedantic -Werror expr.c demo.c -o /tmp/opencode/ch01-demo
/tmp/opencode/ch01-demo
cc -std=c11 -Wall -Wextra -Wpedantic -Werror observations.c -o /tmp/opencode/ch01-observations
/tmp/opencode/ch01-observations
cc -std=c11 -Wall -Wextra -Wpedantic -Werror expr.c stack.c test_stack.c -o /tmp/opencode/ch01-stack
/tmp/opencode/ch01-stack
```

The expression language has one input, literals in 0..255, addition, and
multiplication. Each operation reduces its result modulo 256. It has no calls,
memory accesses, division, exceptions, or nondeterminism. These semantics are
not the semantics of arbitrary C expressions.

Trees must be finite, non-sharing, and acyclic. Binary nodes have two non-null
children. The caller owns every node. Folding changes the tree in place;
disconnected children are not freed, and must remain under caller ownership.
Stack allocation in the demo and arena allocation in the tests satisfy this
contract. Do not treat this as a general heap-allocation API.

`test_expr.c` checks both binary operations on all 65,536 operand pairs,
the worked example for all 256 inputs, repeated folding, and 256 deterministically
generated trees of depth at most four for all inputs. The generated cases are
not exhaustive over trees. It also checks the arithmetic exercise answers and
the proposed add-zero root replacement for both requested left-operand shapes.
Keep assertions enabled when running tests.

`observations.c` is a separate C program. Its expected output is
`value=0 calls=1`; deleting the call would lose the state change.

The book includes `expr.h`, `expr.c`, and `observations.c` directly with
`inputminted`; edit the files rather than retyping their listings in LaTeX.

## Stack-code translation

`stack.h` and `stack.c` add a compiler, a structural verifier, and an executor.
Code has capacity 64 instructions. Compilation accepts valid trees under the
same ownership contract as the evaluator and reports insufficient code capacity.
On failure it restores the logical length, not unused instruction-array bytes.
Compilation can append fragments; whole-program verification requires an empty
initial stack and exactly one final value. The executor verifies on every run.

Pass valid, initialized objects and non-null output pointers. Do not mutate code
between verification and execution. This is not a byte decoder for untrusted
serialized data. Active instruction slots must be initialized.

`test_stack.c` checks all literal operand pairs for both instructions, the exact
worked-example code and its peak stack demand, generated-tree translation before
and after folding for all inputs, malformed-code rejection, and capacity rollback.
The stack compiler/executor listings are also included directly from the C file.

## Wrong-code and reduction laboratory

```sh
cc -std=c11 -Wall -Wextra -Wpedantic -Werror expr.c stack.c fault_lab.c -o /tmp/opencode/ch01-fault
/tmp/opencode/ch01-fault
cc -std=c11 -Wall -Wextra -Wpedantic -Werror trap_lab.c -o /tmp/opencode/ch01-trap
/tmp/opencode/ch01-trap
```

`fault_lab.c` deliberately mutates emitted `MUL8` instructions into `ADD8`.
It is an isolated specimen, not a replacement for the production compiler.
A fixed witness input of 9 exposes a mismatch, and a deterministic greedy
reducer takes the nine-node expression to `0*1` in five accepted steps.
The program checks the exact final shape and compares mutant/production results
for all inputs. It also exhaustively checks the 128 literal pairs for which the
mutation accidentally agrees with multiplication.

The reducer retains valid, successfully compiled and verified wrong-code cases.
It does not count rejection or resource failure as the same failure. Candidate
replacement uses caller-owned nodes and never invokes the mutating folder.
The bounded example's size counters are not an arbitrary-size tree API.
Keep assertions enabled for this laboratory, as for the other test programs.

## Traps, effects, and fuel laboratory

`trap_lab.c` is an isolated specimen with its own `TExpr` type; it does not
change `expr.h`, `expr.c`, or the stack compiler. It adds a division form
that traps on a zero divisor, an event log that records call order, and a
fuel-limited loop that reports exhaustion instead of diverging. The program
checks all 65,536 literal division pairs (256 trap), the always-trapping
fold of `(x+7)/(249+7)`, the safe fold of `(x+7)/(3+5)` with divisor 8 for
all inputs, ordered logs `[1,2]` versus `[2,1]` with the same sum 30, and
fuel boundaries 5-needed/3-fuel failure, 5/5 success, and 0/0 success.
Evaluation is left-to-right with separate statements, and division is
guarded by an explicit zero test so no undefined host division executes.
