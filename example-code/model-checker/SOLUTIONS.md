# Chapter 48 exercise solutions

## 1. Dead ends

A single initial state 0, no explicit edges, and proposition p true has the
implicit edge 0->0. EX p, EG p, and E[true U p] all hold at 0. If p is false,
all three are false. A state labeled `deadlock` before totalization remains
detectable by safety reachability, but a formula inspecting only p cannot tell
an explicit self-loop from an implicit one. The exhaustive tests include both.

## 2. Work the until iteration

For the buggy graph, p is all six states and q={3}. The sequence starting at q is
{3}, {2,3}, {1,2,3}, {0,1,2,3}, {0,1,2,3,4,5}, then unchanged. Thus five
predecessor sweeps establish convergence. In the repaired graph {3} is already
a fixpoint and one sweep suffices. A budget of four on the buggy model returns
`MC_LIMIT` with empty published output even though the fourth approximation is
the exact mathematical answer: the routine has not yet tested equality.

## 3. A safe path is not universal safety

The buggy model satisfies EG !bad at state 0. One witness repeats the cycle
0->1->4->0 forever. Nevertheless 0->1->2->3 reaches bad. Universal safety from
the initial set I is `I & EU(all, bad) == 0`, not membership in EG !bad.
The computed EG set is {0,1,2,4,5}, mask 0x37; the EU set is all states, 0x3f.

## 4. Multi-source shortest trace

Use states 0..3, initial set {0,2}, edges 0->1, 1->3, 2->3, and bad={3}.
The seeded queue is [0,2]. After examining 0 it is [0,2,1]; examining 2 adds 3
with parent 2. Examining 1 must not overwrite parent[3]. The trace is [2,3],
one edge. Enqueuing a state twice can destroy the queue bound and can overwrite
a shortest parent with a longer predecessor. This graph and labeling occur in
the same mathematical family as the exhaustive tests, but have four states;
the argument above is a worked exercise, not a claim that this exact graph is
part of the three-state exhaustive run.

## 5. Budget and array bounds

For the 64-state chain 0->1->...->63 with only 63 bad, BFS needs 64 dequeues
and returns 64 state IDs. With 63 dequeues it returns `MC_LIMIT`, no trace.
EU(all,{63}) grows by one state per sweep: 63 sweeps obtain all states, and a
64th proves equality. EG({0,...,62}) loses one state per sweep: after 63 it is
empty; the 64th establishes equality. These exact cases are executed in
`boundaries()`. Queue capacity is 64 because each state is marked on enqueue.
The sentinel parent is the root itself, so no out-of-range index is required.

## 6. Extending the model's abstraction boundary

Split state 1 into “entry pointer read” and “guard checked” phases. Add an
invalidation edge between the check and the call. Merely deleting 2->3 in the
coarse model does not prove this race impossible: it made guard-plus-entry an
atomic transition. A refined protocol must prevent invalidation during entry,
recheck a generation at a suitable synchronization point, or arrange that old
code stays valid for in-flight callers. Which protocol works depends on the
runtime's memory model and reclamation rules. The lab implements none of these
concurrency mechanisms; this answer identifies the missing states and proof
obligation rather than claiming to verify a concrete concurrent implementation.
