# Architecture exercise solutions

Source IDs refer to filenames, not printed chapter numbers. Numerical examples
are also checked in `test_labs.py`.

## Source 61 — Incremental databases

1. **`ex:ch61-cutoff`:** Initial result is 10. Changing x from 2 to 4 reruns
   parity, which stays zero; the client reuses 10. Changing y to 11 reruns the
   client and returns 11; parity is validated without rerunning its provider.
2. **`ex:ch61-branch`:** The old trace might be `[flag, body]`. After the flag
   becomes false, the new provider reads only flag. The old body may be deleted
   or invalid in the new program. Stop old-trace validation at the changed flag,
   then execute the provider and replace its dependency list.
3. **`ex:ch61-hidden`:** Compile once for CPU A, change the environment to CPU B,
   request the same code key, and receive A's cached instructions. Track the CPU
   configuration as an input or include a complete immutable target description
   in the key. Every provider that depends on it must read through that interface.
4. **`ex:ch61-inverse`:** `2+3` and `1+4` both fold to 5. To obtain 6, the first
   could become `2+4` or `3+3`; alternatively replace the whole expression with
   `6`. Provenance alone does not select one. The update tool needs a policy
   describing the editable source location and how retained source structure is
   preserved or replaced.

## Source 62 — Equality saturation

1. **`ex:ch62-wrap`:** `(250+13) mod 256 = 7`, and `(7*13) mod 256 = 91`.
   Each folded constant has unit tree cost. The original addition has cost 3;
   the nested multiplication has cost 5 before folding.
2. **`ex:ch62-congruence`:** Canonicalizing both child IDs makes both node keys
   `f(find(a))`. Their owner classes must be united and affected parents revisited.
   Updating only the child parent pointers leaves duplicate parent classes and
   stale memo-table keys.
3. **`ex:ch62-cycle`:** No finite tree exists. Every attempted root requires two
   further instances of itself, with no leaf. The class retains infinite cost
   and extraction fails explicitly.
4. **`ex:ch62-effects`:** If `f` prints a character, deleting it deletes output;
   if it diverges, replacing it by zero changes termination. The byte-model rule
   requires a pure, total operand computation and ordinary total multiplication.

## Source 63 — Semantic and effect IRs

1. **`ex:ch63-stores`:** For p=1,q=2, memory ends `{1:9,2:4}` and the load is 9
   in both orders. For p=q=1, original memory is `{1:4}` and the load is 4;
   swapping produces `{1:9}` and load 9. Reject the latter swap.
2. **`ex:ch63-effects`:** Reject: `W_a intersect R_b = {B}`. The second operation
   reads the first operation's output, even though A, B, and C are disjoint.
3. **`ex:ch63-trap`:** `unused = checked_div(1, x); print(7)` prints nothing when
   x=0 if division traps first. Removing the division prints 7. A proof that x
   is nonzero and the operation is otherwise pure, terminating, and nontrapping
   permits removing its dead result computation.
4. **`ex:ch63-summary`:** A caller may have reused a load from A across the call
   under a read-only summary. Writing A invalidates that reuse. The callee's
   return type may stay unchanged, so the caller needs a dependency on the
   effect summary, not only on its type.

## Source 64 — Binary lifting

1. **`ex:ch64-add`:** 128+128=256, so r=0. CF=1, PF=1, AF=0, ZF=1, SF=0,
   OF=1. Unsigned addition carries out; signed addition of -128 and -128 cannot
   fit and produces a nonnegative wrapped result. Low nibbles generate no carry.
2. **`ex:ch64-partial`:** MOV sets AL=7, ADD gives AL=16, and final RAX is
   `0xABCD10`. Both operations write only the low byte.
3. **`ex:ch64-unknown`:** Observations are not an exhaustive target proof.
   Retain an unknown exit, use a sound target analysis, or guard runtime dispatch
   and fall back for other destinations. Deleting it makes unseen inputs wrong.
4. **`ex:ch64-xor`:** AF is unspecified. A contract allowing either value does
   not promise it equals the previous AF. Tests must not require a particular
   AF bit after XOR; operations that later observe it need an explicit model.

## Source 65 — Adaptive interpreters and staging

1. **`ex:ch65-transition`:** cold→integer returns 7; integer→generic returns
   the concatenated string and increments misses to 1; generic remains generic
   and returns 17; Boolean/integer raises TypeError. Misses remain 1.
2. **`ex:ch65-stack`:** `input; const 2; add; const 4; mul`. Stack states at
   x=3 are `[3]`, `[3,2]`, `[5]`, `[5,4]`, `[20]`.
3. **`ex:ch65-effect`:** File contents, errors, timing, and the read event itself
   may be dynamic. Specialization must residualize the operation unless the
   staging contract explicitly supplies immutable static contents and accounts
   for the observable read. Known syntax does not imply a known environment.
4. **`ex:ch65-projection`:** The second projection `S(S,I)` specializes the
   specializer with the interpreter fixed, yielding a compiler. It requires a
   compatible specializer able to specialize its own relevant implementation.
   The companion directly traverses one expression grammar; it has no program
   representation or evaluator for self-application.

## Source 66 — State transfer

1. **`ex:ch66-overflow`:** Fast sums are 7 and 127. At index 2, candidate 128
   fails the narrow range guard. Recover `(i=2,total=127,trace=[(0,7),(1,127)])`.
   Baseline returns total 128 with trace `[(0,7),(1,127),(2,128)]`.
2. **`ex:ch66-error`:** The prefix commits `(0,7)`. At index 1 the type guard
   fails, and baseline reports a type error at 1 with total 7 and only that one
   committed event. The final input 4 is never processed.
3. **`ex:ch66-replay`:** Restarting repeats the first two externally visible
   events. Reaching the same final sum cannot repair the duplicated output.
4. **`ex:ch66-identity`:** Allocate shells a and b, then set `a.next=b` and
   `b.next=a`, and bind the local to a. Require `a.next is b`, `b.next is a`,
   `a.next.next is a`, and `local is a`. Field equality alone misses broken
   sharing; recursive construction without preallocated shells may not terminate.

## Source 67 — Continuous compilation

1. **`ex:ch67-stale`:** The old request carries epoch 0 but the owner is at 1,
   so reject publication. The old acquired version remains retired but live
   until its lease is released; the next collection may then remove it.
2. **`ex:ch67-race`:** Reader loads pointer v; owner retires v and sees count 0;
   collector frees v; reader increments a field in freed memory and enters it.
   Serializing pointer selection and lease acquisition with retirement prevents
   that gap. A different concurrency mechanism must prove an equivalent rule.
3. **`ex:ch67-sparse`:** Payload `{1:4,3:1}`, logical length 5, valid indices
   0–4. Aliases must retain the same handle identity and read the same values,
   including implicit zero at index 4. Index 5 remains out of bounds. Rebuilding
   dense storage produces the original five-element sequence.
4. **`ex:ch67-lattice`:** Faster/larger and slower/smaller versions can be
   incomparable under a multi-objective order, and joins/meets have not been
   supplied. Record derivation, assumptions, or a proved state-transfer edge in
   a typed artifact graph without asserting lattice structure.

## Source 68 — Profiles, learning, and placement

1. **`ex:ch68-threshold`:** `C(p)=31-26p`; `C(p)<10` iff `p>21/26`.
   Endpoint costs are 12.8 and 7.6. A policy requiring improvement throughout
   the supplied interval rejects this candidate.
2. **`ex:ch68-fit`:** Sums are 10,46,30,130; denominator 20; slope 3 and
   intercept 4. Prediction at n=6 is 22. Exact fitting of synthetic samples
   establishes neither hardware accuracy nor generalization beyond that model.
3. **`ex:ch68-order`:** ABC/CBA cost 19, ACB/BCA cost 27, BAC/CAB cost 26.
   All six orders are covered; minimum 19 places B in the middle.
4. **`ex:ch68-device`:** Equality occurs at n=40. At n=20 CPU/device costs are
   80/140; at n=80 they are 320/200. Keeping data resident can remove transfer
   components from the 120 fixed cost and shift the crossover, but requires a
   model that also includes the next consumer and synchronization.
