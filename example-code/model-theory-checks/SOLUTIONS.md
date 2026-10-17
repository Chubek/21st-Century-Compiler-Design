# Worked solutions: Chapters 45 and 46

The state names and edge relations below are those in the chapters. Run
`checks.c` as described in `README.md` to recompute the finite calculations.
Fairness answers additionally use the explicitly stated infinite repetition.

## Chapter 45

### `ex:ch45-cache-witness`

The shortest violating path is `c0,c1,c2,c3,c4,c6`, with **five transitions**.
All earlier states satisfy `pinned => resident`. Self-loops only increase the
length. At `c6`, pinned is true and resident is false. The publication-metadata
invariant still holds: `c3` is the only published state and has metadata; `c6`
is not published. Checking only publication would miss the dangling user.

### `ex:ch45-terminal-policy`

Add `h -> h` and `z -> z`. Add `halt` at `h` and `deadlock,error` at `z`.
At `u`, `EF halt` is true via `u,h`; `AF halt` is false via `u,z,z,...`;
`AG !error` is false via the same branch. Over all states, `EF halt` is `{u,h}`,
`AF halt` is `{h}`, and `AG !error` is `{h}`. A self-loop records the terminal
outcome without equating stuckness with successful termination.

### `ex:ch45-monitor-history`

Starting clear, the monitor states after the four events are
`P,C,P,B`. A flag that remembers only whether any reconstruction ever happened
would remain true after the second failure and incorrectly permit the commit.
The obligation refers to reconstruction **since the most recent failure**.
A subsequent reconstruction leaves the monitor in `B`: the forbidden commit
already occurred and cannot be repaired retroactively.

### `ex:ch45-branching`

After adding `b -> e` and the loop at `e`:

| State | EF done | AG EF done |
|---|---|---|
| a | true | false |
| b | true | false |
| c | true | true |
| d | true | true |
| e | false | false |

The successful path through `c,d` still proves existential completion from
`a,b`. But `e` is now reachable from both and has permanently lost that option,
so continuing availability fails. Before the new edge, retrying never removed
the option even though it could postpone completion forever.

### `ex:ch45-fairness-gap`

On `(uv)^omega`, the action is enabled infinitely often but is not continuously
enabled on any suffix. Weak fairness therefore holds; strong fairness fails
because the action is never taken. If it is enabled at both states, it is
continuously enabled throughout, so both weak and strong fairness fail.
Neither fairness notion imposes a fixed finite response bound.

## Chapter 46

### `ex:ch46-multiple-initial`

The shortest witness is `s4,s5`, with **one transition**, since `s4` itself is
now initial. Searching only from `s0` first can return `s0,s4,s5`, of length
two. Multi-source BFS must seed every initial state at distance zero before
processing any outgoing edge.

### `ex:ch46-until-zero`

For `P={s0,s1}` and `Q={s2,s3}`, the iteration is:

| Round | Set |
|---|---|
| 0 | `{s2,s3}` |
| 1 | `{s1,s2,s3}` |
| 2 | `{s0,s1,s2,s3}` |
| 3 | unchanged |

States `s2,s3` do not belong to `P`. They satisfy until with the target already
true, so the prefix before reaching `Q` is empty. Requiring membership in `P`
at the target would implement a different operator.

### `ex:ch46-eg-pruning`

| State | EG !error | AF installed | AF installed without `s1 -> s1` |
|---|---|---|---|
| s0 | true | false | false |
| s1 | true | false | true |
| s2 | true | true | true |
| s3 | true | true | true |
| s4 | false | false | false |
| s5 | false | false | false |

The safe greatest fixed point starts at `{s0,s1,s2,s3,s4}` and removes `s4`,
which only leads to `s5`; it then stabilizes. The path `s1,s1,...` witnesses
both the possibility of staying safe and the failure of inevitable installation
at `s1`. Removing that loop makes installation inevitable at `s1`, but `s0`
still has the error branch through `s4`. No fairness restriction is being used.

### `ex:ch46-bounded-unsat`

`Phi_k` is satisfiable exactly when `k >= 4`. The bad state is first reached
after four edges and its self-loop extends the execution to every larger bound.
UNSAT at bound three establishes only that there is no bad-reaching prefix of
at most three transitions. It does not establish the invariant. The general
finite-state completeness bound for this five-state invariant query is four.

### `ex:ch46-accepting-walk`

Use the closed walk `h,u,h,v,h`. Repeating its four edges visits both required
sets infinitely often. Each simple cycle, `h,u,h` or `h,v,h`, misses the other
acceptance set and is nonaccepting by itself. This example explains why a
generalized Buechi witness may need a closed walk rather than one simple cycle.

### `ex:ch46-refinement`

For `I,A,D`, concrete endpoint propagation is `{i}`, `{u}`, `{d}`; the trace is
real. Originally, `I,A,E` gives `{i}`, `{u}`, `{}`, so it is spurious.
After adding `i -> w`, the latter calculation becomes `{i}`, `{u,w}`, `{e}`.
The concrete witness is `i,w,e`. Splitting `A` by mode now retains the reachable
route `I,W,E`, so refinement cannot eliminate the real error while preserving
all concrete transitions.

## Additional checked chapter calculations

- Baseline cache: the only state violating `pinned => resident` is `c6`, and it
  is unreachable before adding the faulty reclamation edge.
- Pass graph: `EF done` and `AG EF done` hold everywhere; `AF done` and
  `AX valid` hold at `{c,d}`; `EG !done` holds at `{a,b}`.
- Job graph distances in state-number order are `0,1,2,3,1,2`.
- Main until rounds are `{s3}`, `{s2,s3}`, `{s1,s2,s3}`,
  `{s0,s1,s2,s3}`, unchanged.
- Main greatest-fixed-point rounds for `{s0,s1,s2,s4}` are that initial set,
  `{s0,s1}`, unchanged.
- For all 4,096 pairs of operand sets on the job graph, least-fixed-point
  until agrees with independent path search bounded by five edges.
- For all 64 operand sets, greatest-fixed-point EG agrees with searching for
  a six-edge path entirely within the operand set. Such a path repeats a state;
  conversely, any infinite path has such a finite prefix.
