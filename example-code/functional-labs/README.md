# Functional-language implementation

Original companions for sources 89–93. Python 3.11+, standard library only.

```sh
PYTHONDONTWRITEBYTECODE=1 python3 -m unittest discover -s example-code/functional-labs -v
python3 example-code/functional-labs/fun.py example-code/functional-labs/sum.fun --mode vm
python3 example-code/functional-labs/fun.py example-code/functional-labs/factorial.fun --mode lazy
```

The examples return 20 and 720. Modes are `strict`, `vm`, and `lazy`; the default
is `vm`. All CLI paths validate syntax and infer types before optimization and
execution. Only successful scalar integer/Boolean results are printed. Failures
exit two. The library APIs are for trusted validated syntax; they are not
standalone validators of arbitrary hostile Python objects.

## Language

Atoms: exact integer literals, `true`, `false`, variables, `nil`.
Names contain ASCII letters, digits, and underscores, do not start with a digit,
and cannot be reserved words. Forms:

```text
(lam x body)
(app function argument)
(let x value body)
(letrec f (lam x body) continuation)
(if condition yes no)
(add left right)   (mul left right)   (eq left right)
(cons head tail)
(case list empty-branch (head-name tail-name cons-branch))
```

The two case field binders must be distinct. Recursive bindings require a lambda
right side. The input is exactly one expression; there is no quote, comment,
implicit application, arbitrary constructor declaration, or module system.
Parser budgets are 10,000 tokens and nesting depth 100. Execution has a separate
fuel budget; host recursion/memory and integer-conversion limits can also apply.
This is not a hardened untrusted-input service: large exact arithmetic and
allocation can consume resources not represented by a simple step counter.

## Implemented pipeline

- Finite-type inference for Int, Bool, arrows, and homogeneous List types.
  Schemes support let polymorphism with fresh instantiation and environment-aware
  generalization. Recursive uses within the right side are monomorphic; no
  polymorphic-recursion inference is claimed. Unification has an occurs check.
- Lexical strict interpreter using closures and binding cells. Function and
  argument are evaluated left-to-right; constructor fields are strict.
  A preallocated cell ties recursive closure environments safely for lambda RHSs.
- Structured stack-code compiler/VM. Branch, let, lambda, and case instructions
  retain nested blocks. This is compiled bytecode-like structured code, not
  native code. The VM recursively executes blocks on the host stack and does
  not promise proper tail calls. It checks one result per completed block.
- Binding-aware optimizer folding exact integer primitives, add-zero,
  multiply-one, and Boolean-literal conditionals. It deliberately does not erase
  multiply-zero operands, inline by spelling, reassociate, or guess purity/totality.
- Alternate call-by-need interpreter. Thunks retain call-site environments;
  closures retain definition-site environments. Force updates successful thunks,
  rejects busy reentry as a black hole, and clears obsolete expression/environment
  references. Failure resets the thunk to pending and propagates the host error.
  Lazy Cons fields remain thunks; case demands only outer structure.

Strict and VM modes implement the same source semantics. Lazy mode is an
explicit alternate semantics and can terminate on unused divergent arguments
where strict evaluation does not. Agreement is tested only on the specified
terminating pure programs; equal fuel exhaustion is never called equivalence.
Fuel counts differ between machines, so remaining-fuel values are not comparable
performance measurements.

Memory management is provided by Python, including cyclic environments. No new
collector, native closure layout, concurrent thunk protocol, STG backend, lambda
lifter, general pattern-matrix compiler, or optimal graph reducer is implemented.
The chapters explain those extensions and distinguish them from the executable
core. Exact integer arithmetic is not fixed-width machine overflow arithmetic.

## Verification

Five test methods check:

- list sum, factorial, lexical capture despite shadowing, polymorphic identity,
  Nil/Cons branch binding;
- unbound/type-invalid applications, occurs failure, branch and list mismatch,
  forbidden recursive RHS, malformed/trailing syntax;
- 507 generated higher-order programs: 13×13 operand pairs and three primitives,
  each compared with independent expected arithmetic under source strict,
  compiled strict, optimized compiled, lazy, and optimized lazy execution;
- one-time thunk evaluation, direct black-hole rejection, a deliberate
  strict/lazy termination distinction, and non-erasure of a divergent strict
  multiply-zero operand;
- all three CLI modes returning the expected sum.

`solutions.md` answers all 20 exercises in sources 89–93. The other two labs
provide independent string/FST and rewriting checks used by the expansion.
