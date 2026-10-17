# Functional-language exercise solutions

## 89 — Core semantics

1. f captures the binding x=11. Later x=99 is a distinct binding. f(4) returns
   `11+4=15`. Erroneous caller-environment lookup would return 103.
2. Strict evaluation demands loop(0) before function entry and never produces
   the seven (a bounded run reports exhaustion). Lazy evaluation binds an unused
   thunk and returns seven without entering loop. This is an intentional
   semantic difference, not a compiler disagreement.
3. Closure construction stores its environment without executing its body.
   Reserve cell f, capture it, then fill it with the closure. An eager RHS that
   immediately reads f could instead observe an uninitialized cell; the syntax
   restriction excludes that case. General recursive values need a different
   initialization/demand protocol.
4. The chosen run did not finish within its charged steps. That does not prove
   mathematical divergence; a larger budget could finish. A partial value is
   not a completed result unless the language explicitly defines streaming or
   partial outcomes. The CLI reports failure and emits no success scalar.

## 90 — Types and patterns

1. Parameter x gets fresh alpha; body returns alpha, so identity has
   alpha→alpha. With no free alpha in the environment, let generalizes it to
   forall alpha.alpha→alpha. Instantiate beta→beta at true, solve beta=Bool;
   separately instantiate gamma→gamma at 17, solve gamma=Int.
2. x has monotype alpha. Applying x to x demands alpha=alpha→beta for fresh beta.
   Alpha occurs in its own proposed structure, so no finite type solves it.
3. Nil has List(beta), cons(7,nil) forces beta=Int, and cons(4,...) is List(Int).
   Replacing seven by true forces the tail to List(Bool), conflicting with the
   outer head's Int element requirement.
4. Nil branch returns Int eight, so the cons branch head must be Int too.
   Scrutinee has List(Int), tail has List(Int), result has Int.

## 91 — Closure representations

1. Only x is captured. One flat environment field stores x=11; converted code
   receives env and y and computes env.x+y. Parameter y is not captured.
2. Create `addCaptured(x,y)=x+y`. A direct call of the former local function
   becomes `addCaptured(current_lexical_x,argument)`. Shadowing at a caller
   cannot replace the declaration identity of the required lexical x.
3. `S(K f)I a → (K f a)(I a) → f(I a) → f a`, with permissible redex order
   under these pure combinator rules. Contracting to f at compile time needs
   the chosen evaluation semantics, not just this symbolic reduction.
4. The escaped value must remember which x was supplied and which code remains
   to call. A partial-application object, closure, or equivalent environment
   retains that state. Moving code to top level does not erase captured data.

## 92 — Lazy runtime

1. Bind pending x. First force marks busy, evaluates 18+5 to 23, stores it,
   marks done, and clears old computation references. Second force returns 23.
   Outer add returns 46; exactly one thunk is evaluated for the first time.
2. pending→busy, then recursive force observes busy and reports black hole.
   The reference failure handler resets the original thunk to pending before
   propagating. It does not publish a done value or memoized source exception.
3. Cons already identifies the outer constructor; head and tail are independent
   suspensions. A case binding can demand the head alone. Traversing to make a
   full host list forces all tails and can diverge or retain much more memory.
4. Clear obsolete expression and captured environment after storing the result
   under the runtime's GC/publication protocol. Other closures, roots, or the
   result itself may still reference the input, so local clearing does not prove
   global unreachability. Let the collector determine that from the full graph.

## 93 — Implementation

1. `CONST 4` gives [4]; `CONST 7` gives [4,7]; `ADD` pops right seven and left
   four and pushes eleven, giving [11]. Net result count is one.
2. sum([])=0; sum([5])=5+0=5; sum([2,5])=2+5=7. Only the selected case branch
   runs, and each cons arm binds its head and tail in a fresh lexical scope.
3. Strict evaluation requires evaluating loop(0), which diverges. Returning
   zero removes that required evaluation and changes termination. The optimizer
   retains the multiplication; pure arithmetic does not prove total operands.
4. Define concrete closure/environment layouts, entry/calling conventions,
   argument/return locations, recursive initialization, allocation and GC,
   stack/heap root maps, frame/continuation handling, instruction selection,
   scheduling/register allocation, object emission/linking, and validation.
   Proper tail calls and lazy updates need additional explicit runtime contracts
   if those features are included. Structured VM tests become one semantic
   oracle, not a substitute for native ABI and memory tests.
