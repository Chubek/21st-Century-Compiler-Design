# Solutions for the analysis chapters

Exercise identifiers refer to source labels, so they remain stable when the new
chapters are inserted after the existing abstract-interpretation chapter.
Arithmetic is mathematical integer arithmetic except where modulo 256 is explicit.

## Designing Sound Static Analyses (`ch42.tex`)

### `ex:ch42-square`

Joining the inputs gives `[-3,8]`; the specialized square transfer gives `[0,64]`.
Squaring separately gives `[9,9]` and `[64,64]`; their join is `[9,64]`.
Only the latter excludes zero. The former includes the exact set `{9,64}`, so it
is sound. Its zero comes from the spurious input zero introduced by the join.

### `ex:ch42-wrap`

Inputs 253, 254, and 255 produce 255, 0, and 1. The exact set is `{0,1,255}` and
the ordinary unsigned interval hull is `[0,255]`. Mathematical addition yields
`[255,257]`; input 254 produces zero outside that interval (255 also refutes it).

### `ex:ch42-certificate`

The entry check passes. Transfer produces `[5,5]`, which is not included in the
candidate `[2,4]`. The smallest correct exit interval is `[5,5]`.

### `ex:ch42-dependencies`

Changing `b` invalidates `{b,c}`. It does not invalidate `a`, because a dependency
arrow points toward a reader. After a real but unrecorded dependency `e -> c` is
introduced, changing `e` reaches `d` but misses `c`; the cached `c` can then depend
on obsolete data and be unsound. Register the new dependency during recomputation.

## Relational and Path-Sensitive Analysis (`ch43.tex`)

### `ex:ch43-close`

In order `(0,x,y)`, the initial closed matrix is

```text
 0   0  -2
 4   0  -2
 6   2   0
```

After inserting `y <= 4` and closing:

```text
 0   0  -2
 2   0  -2
 4   2   0
```

Only entries `(y,0)` and `(x,0)` change, from 6 to 4 and 4 to 2 respectively.
The latter follows from `x-y <= -2` and `y <= 4`.

### `ex:ch43-cycle`

The additional lower bound has entry `M[0,x] = -3`, whereas `M[x,0] = 2`.
The two-edge cycle has total weight `-3+2 = -1`, so the conjunction is empty.
A nonempty abstract conjunction might still describe only states introduced by
earlier joins or forgotten operations; its satisfiability is not reachability.

### `ex:ch43-partition`

The joined DBM represents `2 <= x,y <= 9` and `-7 <= x-y <= 7`.
It admits `(2,2)` and `(9,9)`, so the best range of the sum over that DBM is `[4,18]`.
Both separate paths give the singleton sum 11; their output join is `[11,11]`.
The domain cannot encode the shared relation `x+y=11`.

### `ex:ch43-loop`

Entry is `i=0,j=4`. The first header evaluation gives `i in [0,1]`,
`j in [4,5]`, and `j-i=4`.
Immediate widening gives `i in [0,+infinity)`, `j in [4,+infinity)`, and `j-i=4`.
The true guard caps `i` at 6 and hence `j` at 10. Incrementing and joining entry
gives `i in [0,7]`, `j in [4,11]`, still with `j-i=4`.
One replay installs that invariant. The exit guard implies `i=7,j=11`.

The chapter's original loop is the same computation with bound 6 and distance 3:
the widened ranges are `[0,+infinity)` and `[3,+infinity)`, replay yields `[0,6]`
and `[3,9]`, and exit is `(6,9)`. The fixture also enumerates every concrete header
state to check coverage independently of the abstract loop computation.

## Interprocedural and Heap Analysis (`ch44.tex`)

### `ex:ch44-matching`

Site `a` receives 3 and site `b` receives 10. An unmatched graph allows entering
from `a` and returning to `b` with 3. Applying the relation `r=argument+2` to each
actual argument at its own continuation yields the correct separate values.
Keeping the summary relational retains this distinction even when the body and
summary representation are shared.

### `ex:ch44-recursion`

Round 1 adds `(0,2)`, round 2 `(1,3)`, round 3 `(2,4)`, round 4 `(3,5)`, and round 5
`(4,6)`. Round 6 is the first unchanged round. Each round reads only the summaries
from the preceding round. An in-place worklist can discover pairs sooner, so its
visit numbers should not be compared directly with these synchronous rounds.

### `ex:ch44-weak`

The site value set is `{1,6}` and its interval hull is `[1,6]`. Two objects retain
the old value 1. A singleton points-to set identifies one abstract site, not one
concrete object. Replacing the site's value by 6 would lose those two objects.

### `ex:ch44-alias-case`

- `(p,q)=(u,v)`: return 9, since the store changes only `u`.
- `(p,q)=(u,u)`: return 5, since the load observes the store.
- `p=u`, with `q` possibly `u` or `v`: return set `{5,9}`, or interval `[5,9]`.

Reusing the old value through `q` requires proving its accessed cell disjoint from
the store's cell, with compatible lifetime and access semantics. Merely having
different parameter names or different pointer variables does not establish that.

## Additional worked-example results

- Chapter 42: branches `x=-4` and `x=6` yield exact `y` values `{1,11}` and exact
  squares `{16,36}`. The merged square transfer returns `[0,36]`.
- Chapter 43: enumerating `(i,j)=(0,3)..(5,8)` gives every entry of the displayed
  DBM. Guard `j<=6` leaves exactly the first four pairs; adding `i>=4` leaves none.
- Chapter 44: the recursive summary on `0..3` contains exactly `(n,n+2)` and
  stabilizes on round 5. Concrete heap enumeration gives `{0,7}` for the repeated
  site, `{5}` for the definite unique target, and `{2,5}`/`{5,9}` for uncertain
  targets initially containing 2 and 9.
