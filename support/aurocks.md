# Aurocks: Perl-to-C99 backtracking parser generator

Requires Perl 5.10+ and its core
`FindBin` module, and a C99 compiler. No external regex library is linked.
Keep `aurocks.pl`, `aurocks-emit.pl`, and `aurocks-runtime.c` together.
Generated C files are standalone. See source chapters 49–52.

```sh
perl support/aurocks.pl example-code/aurocks/calculator.g > /tmp/opencode/calc.c
gcc -std=c99 -Wall -Wextra -Wpedantic -Werror /tmp/opencode/calc.c -o /tmp/opencode/calc
/tmp/opencode/calc '2+3*(4+1)'
PYTHONDONTWRITEBYTECODE=1 python3 -m unittest discover -s support -p test_aurocks.py -v
```

## Grammar and choice semantics

Optional `%{ C prologue %}`, then `%start Name` and optional `%skip /regex/`,
then `%%` on its own line, productions, a second `%%` line, and C epilogue.
Prologue placement is **after public runtime type definitions**, before runtime
functions. Actions may call functions declared there and defined in the epilogue.
The separator lines must not appear inside C text. The reader is not a preprocessor.

Rules use `Name: alternative | alternative;`. Alternatives contain named
nonterminals, single/double-quoted byte strings, slash-delimited regexes, or
`%empty` alone. C actions in balanced braces are optional and occur only at the
end of an alternative. Strings and comments protect their braces and dollar signs.
Grammar comments use `//` or `/* ... */`; `/` inside a regex must be escaped.

Alternatives retain their source order. A successful callee offers its candidate
to a continuation that includes the caller's remaining symbols and the final
end-of-input check. Failure there reopens the callee's next alternative.
The first **complete** parse wins. This is neither PEG local ordered choice nor
GLR/GLL; there is no chart, memoization, forest, or ambiguity enumeration.

Every regex/literal terminal chooses its longest matching prefix **atomically**.
It does not enumerate shorter matches. Thus `S: /a+/ 'a';` rejects `aa`.
There is no global lexer priority: the grammar requests individual terminals.
Layout is repeatedly skipped before terminals and at complete-input acceptance.

## Supported regex subset

Byte literals, escaped punctuation, `\n \r \t \f \b \a \v \0`, `.`, classes,
negated classes, ascending ranges, groups, `|`, `* + ?`, and `{m}`, `{m,n}`,
`{m,}`. Dot excludes newline. ASCII POSIX classes include `alnum alpha blank
cntrl digit graph lower print punct space upper xdigit`, plus `word`.
Outside classes, `\d \D \s \S \w \W` are supported; inside classes the positive
`\d \s \w` forms are supported. Leading `^` means the current match start;
`$` tests the absolute supplied buffer end. This is not full Perl/POSIX syntax:
no lookaround, captures, backreferences, lazy quantifiers, or Unicode properties.
Unknown alphanumeric escapes, reversed ranges, and stacked quantifiers are errors.

Terminals and layout must be nonnullable. A conservative structural nullability
check treats end assertions as nullable, so an impossible nullable-shaped regex
may also be rejected. Empty grammar behavior belongs in `%empty`.
The alphabet is bytes; explicit lengths allow embedded NUL. No implicit decoding,
locale folding, or newline normalization occurs.

## Static validation and limits

The generator rejects undefined symbols/start, unsupported directives, malformed
tokens, invalid action references, and left recursion through any nullable prefix.
It does not rewrite left recursion automatically. Upper capacities are 128
nonterminals, 512 alternatives, 64 RHS symbols, 1024 NFA states per terminal,
and counted repeat bounds of 256. These are implementation limits, not claims
about the language class. Emission is deterministic and occurs after validation.

## Generated API and ownership

```c
AUResult parse_Name(const char *input, size_t length, const AULimits *limits);
void au_result_free(AUResult *result);
```

`--entrypoint NAME` replaces `parse_Name`. There is no `yyparse` alias.
The input pointer must be nonnull and address at least `length` readable bytes;
it remains valid throughout the call. The result contains:

- `status`: `AU_OK`, `AU_SYNTAX`, `AU_LIMIT`, `AU_NOMEM`, or `AU_INVALID`.
- `value`: action result; NULL is allowed on success.
- `offset` and `expected`: furthest failed terminal-start/end-check offset and
  one expectation (first retained on ties). Cleared expectation on success.
- `steps`: charged sequence/choice entries and NFA rounds.
- `blocks`: result-owned storage; callers do not manipulate this field.

Default limits are 1,000,000 charged steps and 256 active engine frames. Depth
above 256 is invalid; zero budgets deliberately yield `AU_LIMIT`. Frame depth
is not source nesting depth. `AU_LIMIT` is not evidence of syntax rejection.
Each NFA round scans its active states, so steps do not count CPU instructions.
Semantic actions and arbitrary C work are outside the search budget.

Actions run postorder only on the selected complete derivation. Terminal child
values are nul-terminated copies; `$TOKEN_LEN` preserves the direct last terminal's
byte length. `$TOKEN` is NULL with length zero when there is no direct terminal.
`$n` selects an RHS child's `void *` value; `$$` sets the current result.
Default unary actions forward the child; other defaults return NULL.
Use `au_alloc(p, n)` for values with result lifetime; check for NULL before writing.
On failure, stop that constructor. The evaluator detects allocation failure before
executing parent actions. External effects of already committed actions are not
transactional. Values allocated by other means need an application ownership policy.
Never return a pointer into an action-local stack variable.

Always call `au_result_free`, on success or failure, after consuming/copying the
value. Result storage must not be freed twice through shallow copies of `AUResult`.
Repeated freeing of the same cleared result object is harmless.

## Changes from the supplied implementation

The regex AST/NFA construction is retained, with validation fixes. The grammar
reader now consumes all syntax, protects strings/comments, and supports nested
action braces. Full continuation-based backtracking replaces local commitment.
Actions are deferred; terminal values are populated; speculative token allocation
is eliminated. Silent no-op GLR/SPPF/precedence directives, ad-hoc application type
rewrites, and direct-left-recursion rewrites are replaced by explicit contracts
or rejection. The API now returns status and ownership information, replacing the
ambiguous `void *parse_name(const char *)` interface. Update callers accordingly.
Other copies under `PebbleIR/third_party/` are separate vendored implementations.
