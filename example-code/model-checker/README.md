# Developing a Model Checker (source ch48)

A self-contained C11 finite-state checker. No external C dependencies. Requires
a C11 implementation with `uint64_t`; the test runner requires a POSIX shell,
GCC, and Clang. Keep assertions enabled in the tests. There is no generated code,
heap allocation, recursion in the checker, or global mutable state.

## Build and run

From the repository root:

```sh
mkdir -p /tmp/opencode/model-checker
gcc -std=c11 -Wall -Wextra -Wpedantic -Werror -O2 \
  example-code/model-checker/checker.c example-code/model-checker/main.c \
  -o /tmp/opencode/model-checker/checker
/tmp/opencode/model-checker/checker example-code/model-checker/stale.graph
/tmp/opencode/model-checker/checker example-code/model-checker/fixed.graph
sh example-code/model-checker/test.sh
```

The first model intentionally exits 1, the second 0. The CLI checks proposition
bit 0 as `bad`, prints a shortest state-ID trace on failure, and prints the
satisfaction sets of `EX bad`, `E[true U bad]`, and `EG !bad` as hexadecimal
state bitsets. Optional second argument: maximum BFS dequeues (default 64).
CLI exit codes are 0 safe, 1 unsafe, 2 invalid input/usage/I/O, 3 resource limit.
CTL operations use an independent `n+1`-round budget.

Expected stale output:

```text
unsafe
trace: 0 1 2 3
EX bad: 0x000000000000000c
E[true U bad]: 0x000000000000003f
EG !bad: 0x0000000000000037
```

Expected repaired output:

```text
safe
EX bad: 0x0000000000000008
E[true U bad]: 0x0000000000000008
EG !bad: 0x0000000000000037
```

## Graph format and semantics

Whitespace-delimited unsigned decimal integers, in this order:

1. `n k m`: number of states, initial states, and edge records.
2. `k` distinct initial state IDs.
3. `n` 64-bit proposition masks, in state-ID order.
4. `m` source/destination ID pairs.
5. Whitespace and EOF; no extra tokens or comments.

States are numbered `0..n-1`; `1 <= n <= 64`, `1 <= k <= n`, and
`0 <= m <= 4096`. Duplicate edges are accepted and coalesced; duplicate initial
IDs are invalid. Signs, hexadecimal, suffixes, values exceeding `UINT64_MAX`,
out-of-range IDs, truncation, and trailing data are invalid. A representable
header exceeding the state or edge capacity returns `MC_LIMIT`, without
consuming the remaining file. A malformed header returns `MC_INVALID` first.
The stream must be finite; byte count, wall time, and blocking I/O are not
budgeted. Leading zeroes and all C whitespace are accepted.

Every dead end has an implicit self-loop. No input edge is inserted or mutated.
This total relation gives infinite-path CTL semantics: `EX p` at a dead end is
`p`, and `EG p` at a dead end is also `p`. To detect deadlock itself, label a
state before totalization. The transition system does not impose fairness.

## Public API and ownership

`checker.h` is the complete interface. Graphs and traces contain inline arrays;
the caller owns every object, the API never retains pointers, and there is
nothing to free. Distinct input/output objects must not overlap. Pass valid
live C objects (validation cannot make an arbitrary pointer safe). Inputs are
read-only, except the stream cursor. Unused array slots are ignored.

* `mc_validate(g)`: checks nonzero state count, nonempty and in-range initial
  set, and in-range successor masks. More than 64 states returns `MC_LIMIT`.
* `mc_read(in, out)`: validates into a temporary graph and publishes it only
  on `MC_OK`. The supplied output is zeroed on any failure. Does not close `in`.
* `mc_universe(n)`: low `n` bits, including the special 64-bit case; zero for
  out-of-range `n`. Never evaluates a shift by 64.
* `mc_atoms(g, mask)`: states where **any** proposition bit in `mask` holds.
  Returns zero for invalid graphs; call `mc_validate` separately when the
  distinction between error and the empty set matters. A zero mask matches none.
* `mc_safety(g, bad_mask, max_visits, out)`: multi-source BFS. `MC_UNSAFE`
  supplies a shortest trace in number of edges, including both endpoints.
  An initially bad state has a one-state trace. Initial states and successors
  are considered in increasing ID order, making ties deterministic.
  `MC_SAFE` means all reachable states have been checked. `MC_LIMIT` means
  the next queued state could not be examined. No partial trace escapes.
  `out` is zeroed on all results except `MC_UNSAFE`.
* `mc_ctl(g, op, p, q, max_rounds, out)`: `p` and `q` are **state sets**,
  not proposition masks. Evaluates EX(p), E[p U q], or EG(p); `q` is ignored
  for EX/EG but still must be a valid state set. Each round performs one
  predecessor sweep; EX takes one, EU/EG take at most `n+1`, including the
  final equality check. `MC_OK` publishes the complete satisfaction set over
  **all** states, reachable or not. `MC_LIMIT` never publishes an approximation.
  Output is zero on all failures, including invalid operator/set operands.
* `mc_status_name`: stable printable status strings.

All status-returning operations reject null required arguments. The only
`MC_SAFE`/`MC_UNSAFE` producer is `mc_safety`; CTL and parsing return `MC_OK`
on success. Resource exhaustion is a result, never evidence of safety.

State formula composition uses intersection, union, and universe-masked
complement on the returned sets. A formula holds in the model when every
initial state is in its set. The library supplies a CTL operator kernel, not a
formula parser, LTL checker, fair-path checker, or general theorem prover.
EU/EG do not return temporal witnesses; shortest traces belong to safety BFS.

## Compiler-cache case

| State | Meaning | Propositions |
|---|---|---|
| 0 | Interpreted execution, no optimized entry | none |
| 1 | Optimized entry published, assumption valid | none |
| 2 | Assumption invalidated; entry still present | none |
| 3 | An unchecked call entered stale optimized code | bad (bit 0) |
| 4 | Valid optimized call completed | completed (bit 1) |
| 5 | Guard rejected entry; deoptimized call completed | completed (bit 1) |

Edges: 0->1 compile/publish, 1->2 invalidate, 1->4 invoke valid code,
2->3 unchecked stale invocation, 2->5 guard/deoptimize, and 4->0/5->0 retire.
State 3 is an absorbing error via the dead-end convention. Both invocation
paths are possible in the buggy model; the repaired model removes 2->3,
representing a mandatory validity guard before entry. State 3 remains in the
file, unreachable from initial state 0. The trace is a replayable **model**
execution; replay against a real VM requires a separately validated adapter.
The model says nothing about concurrent publication, memory ordering,
in-flight invalidation, or the correctness of deoptimization state maps.

## Verification evidence

Run `sh example-code/model-checker/test.sh`; override `OUT_DIR` if desired.
All binaries and captured CLI output go to `/tmp/opencode/model-checker` by
default. Temporary streams use `tmpfile()` (closed by each test; the runner
also sets `TMPDIR`). The runner compiles both CLI and tests using GCC and Clang,
each with `-std=c11 -Wall -Wextra -Wpedantic -Werror`, at `-O2` and at
`-O1 -g -fsanitize=address,undefined -fno-omit-frame-pointer`.

Each configuration checks:

* All **530** directed graphs on 1, 2, and 3 labeled states, including loops
  and dead ends: `2^1 + 2^4 + 2^9`.
* **28,868** safety queries: every nonempty initial set and every bad-state set.
  Floyd--Warshall distances independently determine reachability and shortest
  length; every returned trace is replayed edge by edge.
* **41,360** CTL queries: all EX/EG operands and all EU operand pairs. EU is
  checked by enumeration of simple witness paths, EG by enumeration of paths
  with `n` edges wholly inside the operand. These do not use fixpoint iteration.
* Twenty malformed streams, duplicate initial IDs, accepted duplicate edges,
  maximum decimal integer and overflow, null outputs, invalid masks/operators,
  capacity errors, and transactional output behavior.
* A 64-state chain: 63-edge trace, bit 63, exact success/failure budget
  boundaries, initially bad state, all states initial, `SIZE_MAX` budgets.
* A parsed complete 64-state graph with all 64 initial states and 4096 edges.
* Both case-study files, all six expected CTL sets, CLI exit codes and invalid
  command-line budgets.

The committed tests were run successfully with GCC 16.2.1 (20260810) and
Clang 22.1.8. ASan/UBSan reported no errors. This is finite experimental evidence,
not a proof of the C implementation or exhaustive coverage of 64-state graphs.

## Files

`checker.h` / `checker.c`: library; `main.c`: CLI; `test_checker.c`:
independent oracles and boundary tests; `test.sh`: build/test driver;
`stale.graph` / `fixed.graph`: case models; `SOLUTIONS.md`: exercise answers.
