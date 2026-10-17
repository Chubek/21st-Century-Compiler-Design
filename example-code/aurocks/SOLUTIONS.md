# Worked answers (source chapters 49–52)

## Regular languages and lexer construction

1. `acbd`: subsets `{0}`, `{1,2}`, `{2}`, `{2}`, `{3}`; accepted.
   `acb` ends in `{2}`, not accepting.
2. Write Q for merged B,C. Distinguishing suffixes: A/Q: `d`; A/D: empty;
   A/Z: `ad`; Q/D: empty; Q/Z: `d`; D/Z: empty.
3. Successful `a` inspections: `5+4+3+2+1=15`. Failed/end checks add constant
   work per attempt but do not change the quadratic family.
4. Empty suffix already distinguishes two different accepting token outputs.
   An accept/reject partition loses that observable distinction.

## Predictive parsing

1. `[id,id,id]` expands S, nonempty I, nonempty T twice, then empty T: five
   productions. It matches seven source tokens plus the end marker: eight.
2. FIRST(A)={id}, nullable(A)=true, FOLLOW(A)={id}. Both alternatives select
   `id`; table cell (A,id) conflicts. The whole language is `{id, id id}`.
3. Left grouping: `(30-8)-5=17`; right grouping: `30-(8-5)=27`. Update
   `left=Sub(left,nextOperand)` after each operator/operand pair.
4. The second comma fails the expected identifier. A placeholder must discharge
   that obligation so that the caller's tail can consume the comma. Retrying
   the same identifier at the same comma makes no progress. The tree remains invalid.

## Backtracking and generalized parsing

1. `ab` chooses A→a; `abb` chooses A→ab; both run the chosen A and S actions,
   twice total. `abx` has no complete derivation and runs zero actions.
2. B can return without consuming; the following A recurs at the original
   offset. The nullable-prefix left-corner graph includes A→A.
3. Sixteen deepest histories for four `a` bytes. Endpoint-only recognition does
   not identify which direct/indirect production built each child or action value.
4. C0 contains S'→•S, S→•AB, A→•, S→A•B, B→•, S→AB•, and S'→S•,
   all with origin 0. The last item accepts empty input. Repeated closure handles
   completed nullable items even when their waiting callers appear later.

## Building Aurocks

1. S→A and S→B (because A is nullable), and B→S. Thus S,B form a cycle.
2. `S: %empty { $$ = NULL; };` succeeds on empty input with AU_OK and a null
   value. Check status and release the result; a null value is not rejection.
3. `S: A 'a'; A: 'a' A | 'a';` uses consuming literal terminals. On `aa`,
   retrying A's base alternative leaves the final byte for S. It accepts a^n
   for n>=2, subject to the engine budget.
4. No action runs until complete syntax succeeds. Once postorder evaluation
   begins, an action can print before a later allocation fails; the runtime
   releases managed allocations but cannot retract that output.
