%{
#include <limits.h>
#include <stdio.h>
static void *number(AUParser *, long);
%}
%start Expr
%skip /[ \t\r\n]+/
%%
Expr: Term Add { $$ = number(p, *(long *)$1 + *(long *)$2); };
Add: '+' Term Add { $$ = number(p, *(long *)$2 + *(long *)$3); }
   | %empty { $$ = number(p, 0); };
Term: Atom Mul { $$ = number(p, *(long *)$1 * *(long *)$2); };
Mul: '*' Atom Mul { $$ = number(p, *(long *)$2 * *(long *)$3); }
   | %empty { $$ = number(p, 1); };
Atom: /[0-9]/ { $$ = number(p, $TOKEN[0] - '0'); }
    | '(' Expr ')' { $$ = $2; };
%%
static void *number(AUParser *p, long value)
{
    long *v = (long *)au_alloc(p, sizeof *v);
    if (v) *v = value;
    return v;
}
/* Demonstration domain: at most nine one-digit leaves. Even 9^9 < LONG_MAX
   on every C99 implementation (LONG_MAX >= 2147483647). */
int main(int argc, char **argv)
{
    if (argc != 2) return 2;
    unsigned digits = 0;
    for (const char *s = argv[1]; *s; ++s)
        if (*s >= '0' && *s <= '9') ++digits;
    if (digits > 9) return 2;
    AUResult r = parse_Expr(argv[1], strlen(argv[1]), NULL);
    if (r.status == AU_OK) printf("%ld\n", *(long *)r.value);
    else fprintf(stderr, "parse status %d at byte %zu\n", (int)r.status, r.offset);
    int status = r.status == AU_OK ? 0 : 1;
    au_result_free(&r);
    return status;
}
