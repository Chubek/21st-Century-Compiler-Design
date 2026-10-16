# PebbleIR teaching implementation

C11, standard library only. This is the executable companion to
`book/pamphlet.ltx`, **PebbleIR: Design, Development and Dogfooding**.
Run commands from the repository root, with assertions enabled:

```sh
cc -std=c11 -Wall -Wextra -Wpedantic -Werror -O1 -g \
  example-code/pebbleir/pebbleir.c example-code/pebbleir/test_pebbleir.c \
  -o /tmp/opencode/pebble-tests
/tmp/opencode/pebble-tests

cc -std=c11 -Wall -Wextra -Wpedantic -Werror -O1 -g \
  example-code/pebbleir/pebbleir.c example-code/pebbleir/demo.c \
  -o /tmp/opencode/pebble
/tmp/opencode/pebble '(x+7)*(3+5)'
```

The demo writes parseable optimized IR to stdout and
`x=9 result=128 vm-dispatches=6` to stderr. Repeat tests with GCC and Clang;
add `-fsanitize=address,undefined -fno-omit-frame-pointer` for instrumented runs.

## Implemented scope

- One function, one unsigned byte input, byte return; `u8` and boolean `i1`.
- Modulo-256 addition/multiplication, unsigned comparison, typed constants.
- Reachable CFGs, jumps, branches, two-predecessor phi prefixes, loops.
- Stable nonzero value IDs, tombstones, bounded builders and revision tracking.
- Structural/type/dominance verification and derived predecessor/dominator sets.
- Canonical versioned text printing, transactional parsing, round-trip tests.
- Expression source parser: `x`, decimal bytes, parentheses, `+`, `*`.
- Transactional replace-all-uses, literal folding, conservative demand-based DCE.
- IR interpretation and lowering to a register VM with two-phase edge copies.

There are no module symbols, calls, memory operations, automatic conversion of
mutable variables to SSA, physical register allocation, SCCP, or implemented
test reducer. Chapters explicitly distinguish these extensions from the working
path. No third-party library is required.

## Contracts

`Pir` owns up to eight blocks and 128 allocated value slots. Entry is block zero;
zero is **not** a value ID. Allocation failure is `PIR_BLOCKS` for blocks and zero
for values. All pointers passed to APIs must be valid and non-null, strings must
be terminated, and clients must exclusively own objects while mutating them.
Public structs permit incomplete construction states. Verify before consuming
them; the interpreter, optimizer, replacement routine, printer, and lowerer do so.

Builders make bounded local edits, not semantic proofs. `pir_replace` verifies
structural validity but the caller must prove value substitution legal.
`pir_compile`, `pir_parse`, `pir_replace`, `pir_optimize`, and `pir_lower` leave
their destination unchanged on failure. Stream parsing still consumes input;
printing may leave a partial stream on I/O failure.

Within a function lifetime, API edits advance a checked revision. Raw construction
backpatches in fixtures advance it explicitly before publishing any facts.
`PirFacts` belongs to its recorded owner and revision. `pir_init`, successful
parsing, and successful compilation begin a new logical lifetime: discard all
old facts and ID associations even when the same memory address is reused.
Copying a `Pir` does not retag an existing `PirFacts` for the new owner.

The reference interpreter counts block entries. The VM counts instructions.
Fuel exhaustion is a harness result, not proof of divergence. The VM performs
runtime bounds checks but is not a complete verifier for arbitrary serialized
bytecode; its intended input is the lowering output. All fixed arrays and
transaction copies consume caller stack or object storage; size workspaces
appropriately on small-stack hosts.

## Test coverage

All 256 input bytes are checked on the diamond, summation loop, and parallel-swap
loop through original/optimized IR and original/optimized VM, against direct
oracles. Another 128 generated depth-bounded source trees run on all byte inputs
against a separate tree evaluator, both IR versions, and optimized VM.

Additional tests cover text round trips, malformed graph/value/type cases,
transaction rollback, full value capacity, revision exhaustion, parser rejection,
loop and VM fuel boundaries, precedence, complete constant folding, and a bad VM
register. The running example's target instruction count is checked as 8 before
optimization and 6 afterward. These are bounded checks, not a proof over all IRs
or a wall-clock performance benchmark.
