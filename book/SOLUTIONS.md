# PebbleIR exercise solutions

These answers use the semantics and capacities of the companion implementation.
They cover all 32 exercises. Labels identify exercises independently of rendered
chapter and page numbers. Executable checks are recorded in `VERIFICATION.md`.

## 1. Requirements

**`ex:pebble-wrap`.** The right factor is `(3+5) mod 256 = 8`.
At x=0, the left factor is 7 and the result is 56. At x=9 it is 16 and the result
is 128. At x=250, `(250+7) mod 256 = 1`, so the result is 8. Reducing only the
final mathematical product also gives 8 here, but the IR's specified intermediate
value is 1, not 257.

**`ex:pebble-capacity-contract`.** Three new values require three slots. With one
slot available, the first constant could succeed before the second allocation
fails. Checking only the final result would discover failure after an observable
partial edit. A whole-expression transaction can construct into a copy and commit
only after all allocations and verification succeed. Alternatively, reserve the
required capacity first, or document that the low-level builder is partial and
requires the caller to discard its incomplete construction. The source compiler
uses the temporary-object contract.

## 2. Semantics

**`ex:pebble-swap`.** Parallel assignment from (1,2) produces (2,1), then (1,2).
Sequential `a=b; b=a` produces (2,2) on the first iteration and remains (2,2).
The first bad state is the second write of the first sequential iteration: it
reads the new a instead of the old a.

**`ex:pebble-fuel`.** Exhaustion at 5 or 19 proves only that the configured
transition budget was insufficient for that run. Neither result proves
divergence. A successful return at 20 establishes termination of this particular
execution with that result and count; it says nothing about other inputs without
an additional argument.

## 3. Object model

**`ex:pebble-id-owner`.** The integer 3 contains no owner identity. In each
function it indexes a different local table; a mistaken foreign ID may even
refer to a well-typed, dominating local definition and escape structural checks.
Use an explicit owner in handles, or enforce ownership at the API boundary.

**`ex:pebble-compact`.** The map is 1→1, 2→2, 3→3, 6→4, 7→5. The old value 7
becomes value 5 with operands 3 and 4. Its return use also changes from 7 to 5.
All external handles and facts for the old identity space must be retired or
translated through the same map.

## 4. Operations

**`ex:pebble-bool`.** Unsigned 7 is less than unsigned 9, so the replacement is
`const i1 1`. A `const u8 1` has the same numeric payload but the wrong type for
the branch contract. Types express which operations may consume values, not
only how many bits happen to be nonzero.

**`ex:pebble-sub`.** For (0,1), `0+256-1 = 255`, yielding 255. For (255,255),
the intermediate is 256, yielding zero after reduction modulo 256. Over byte
inputs the complete expression ranges from 1 through 511; the partial addition
`a+256` ranges from 256 through 511, and subtraction cannot underflow. These
ranges fit the minimum C unsigned range. This is an extension calculation,
not an implemented subtraction opcode.

## 5. Storage

**`ex:pebble-revision`.** The fact cannot be reused under the documented uniform
policy. A constant-propagation cache saying “value 6 is 8” becomes wrong if that
constant changes to 9. Dominance may happen to remain correct, but exploiting
that requires an explicit preservation policy rather than ignoring revisions.

**`ex:pebble-generation`.** A client retains ID 12 for an addition; deletion and
reuse install an unrelated constant at slot 12. The stale ID silently names the
new value. A generation field attached to both the slot and handle distinguishes
the old `(12,g)` from `(12,g+1)`. Generation exhaustion also needs a policy. The
current implementation avoids reuse entirely within a lifetime.

## 6. Construction

**`ex:pebble-builder-order`.** Installing a terminator seals ordinary emission
into that block under the builder convention. This catches accidentally appending
an instruction after control has been finalized. Replacing a terminator is an
intentional edit, advances the revision, and must be followed by verification
before consumption. The low-level terminator setter does not itself prove the
new edge structure valid.

**`ex:pebble-pending-phi`.** Operand zero fails the live-value-ID check. It is a
sentinel, not a literal. Constant zero requires a live, typed `const` record.
Treating unresolved references as numeric zero could change loop recurrences
while making incomplete construction appear valid.

## 7. Text

**`ex:pebble-text-fold`.** The canonical output is:

```text
pebble 1 1 7
v 1 0 input u8 0 0 0 0 0
v 2 0 const u8 0 0 7 0 0
v 3 0 add u8 1 2 0 0 0
v 4 0 dead u8 0 0 0 0 0
v 5 0 dead u8 0 0 0 0 0
v 6 0 const u8 0 0 8 0 0
v 7 0 mul u8 3 6 0 0 0
t 0 ret 7 0 0
end
```

There is a newline after `end`. The serialized allocation count remains seven.

**`ex:pebble-roundtrip-contract`.** No. An interpreter that implements multiplication
as addition could read and execute a perfectly stable snapshot incorrectly.
Likewise, a printer/parser pair could agree on a mistaken field convention.
Round trips check a representation property, not the whole semantics.

## 8. Verification

**`ex:pebble-welltyped-bad`.** In the executable diamond, change value 5's first
operand from entry input 1 to right-arm value 7. Both are `u8`, but value 7's
block 2 does not dominate value 5's block 1. `available` rejects it during
operand validation. The malformed test exercises this edit.

**`ex:pebble-phi-use`.** The incoming phi operand is read at the latch terminator,
where the latch value has already been defined. Its later static ID is therefore
permitted. An ordinary header addition would read before the body executes on
first entry; the latch block does not dominate the header, so the same operand
is unavailable at that ordinary use.

## 9. Graphs

**`ex:pebble-shortcut`.** Adding a direct 0→3 edge leaves `dom(3)={0,3}`. However,
the predecessor set becomes `{0,1,2}`, so the old two-input phi lacks a value for
the new edge. This graph exercise also exceeds the current two-successor entry
terminator and two-input phi representation. Implementing it requires graph
normalization or a representation extension, not merely appending one phi field.

**`ex:pebble-unreachable-cycle`.** Let entry 0 return, with separate edges 1→2
and 2→1. Then `pred(1)={2}` and `pred(2)={1}`. Both have in-degree one, but entry
reaches neither. A traversal from 0, unlike an in-degree test, detects both as
unreachable.

## 10. SSA

**`ex:pebble-sum-trace`.** At x=0, header state `(i,s)=(0,0)` fails the comparison,
the body executes zero times, and the result is zero. At x=4, header states are
`(0,0), (1,0), (2,1), (3,3), (4,6)`. Four body executions produce return 6. The
fifth header visit performs the final failed test.

**`ex:pebble-sibling-stack`.** After visiting the left arm, its definition must
be popped before traversing the right arm in the dominator tree. If retained,
the right use is renamed to the left-only definition instead of the entry
definition. It is then unavailable on the right execution path and violates SSA
dominance. Traversing source siblings is not a substitute for restoring the
dominator-tree traversal state.

## 11. Rewriting

**`ex:pebble-valid-wrong`.** Replace uses of input 1 by the existing entry constant
7 at value 2. The replacement is byte-typed and dominates the original uses.
At x=0 the original returns 56, whereas the rewritten program computes
`(7+7)*8 mod 256 = 112`. A structurally accepted replacement is not an equality
proof. (The now-unused input may subsequently be deleted.)

**`ex:pebble-fold-fields`.** Value 6 keeps its ID, block 0, and type `u8`. Its
opcode becomes `const`, immediate becomes 8, and operand fields a,b and
predecessor fields p,q become zero. The owning function's revision must advance
at the transaction boundary; dependents still refer to ID 6.

## 12. Interpretation

**`ex:pebble-loop-budget`.** There is one entry visit, x body visits, x+1 header
visits, and one exit visit: `2x+3` total block entries. At x=3 this is nine. The
test checks exhaustion at eight and successful result 3 at nine. For all byte
inputs, the maximum is 513 at x=255.

**`ex:pebble-zero-registers`.** Suppose a join directly returns a left-only value
that should have been selected by a phi. Taking the right path leaves that slot
unwritten; zero-filled host storage then supplies a plausible but unrelated zero.
Initialization makes the host read defined, not the IR computation meaningful.
The dominance check rejects this graph before execution.

## 13. Passes

**`ex:pebble-failed-pass`.** A failed transaction retains the original function,
revision, and lifetime; previously valid facts still describe that unchanged
object. Successful parsing replaces the logical lifetime even at the same
address and discards all old facts. A coincidentally equal numeric revision does
not preserve identity across this transition.

**`ex:pebble-three-counts`.** Before optimization: seven allocated slots, seven
live values, eight VM instructions. After: seven allocated slots, five live
values, six VM instructions. The return contributes one target instruction but
does not allocate an IR value slot.

## 14. Optimization

**`ex:pebble-fold-count`.** The subexpressions become 5 and 9, and their product
becomes byte constant 45. Demand retains only the final constant; the unused
input and all other definitions become tombstones. There is one live value and
two VM instructions, constant materialization and return. The test checks this
result and count.

**`ex:pebble-root`.** The condition determines whether another iteration executes,
including whether execution ever reaches the return. Deleting it or replacing
it without proof can change termination even when every terminating execution
returns the same constant. Control operands are therefore demand roots.

## 15. Lowering

**`ex:pebble-copy-bundle`.** Emit `sa=b; sb=a; sc=a; a=sa; b=sb; c=sc` with fresh
scratch registers. Capture gives `(sa,sb,sc)=(9,4,4)`. Publishing gives
`(a,b,c)=(9,4,4)`. Reading old a for both destinations before overwriting it is
essential.

**`ex:pebble-fixup`.** Change the jump operand from temporary block identifier 3
to final instruction index 27. Block identities and instruction positions are
different namespaces. A block can emit several instructions, and earlier edge
stubs can shift every later block's instruction position; interpreting 3 as a
final address would jump into unrelated code.

## 16. Dogfooding

**`ex:pebble-precedence`.** `2+3*4` returns 14; `(2+3)*4` returns 20. Both optimize
to one live constant and a two-instruction VM. The tests check these examples.
Generated trees are printed with explicit grouping and evaluated directly from
their tree shape, providing an oracle independent of parser grouping decisions.
The unparenthesized directed example specifically checks precedence, which a
fully parenthesized generator alone would not exercise sufficiently.

**`ex:pebble-reduction-signature`.** For `2*3`, correct multiplication gives 6
and faulty addition gives 5. For `0*1`, they give 0 and 1. Both are valid wrong-
result witnesses. Literal 6 returns 6 in both implementations because it never
executes the defective multiplication translation. This is a hypothetical
mutant illustrating the proposed reduction workflow; no reducer is claimed as
implemented in the library.
