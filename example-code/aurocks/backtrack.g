%{
#include <assert.h>
#include <stdio.h>
static unsigned actions;
%}
%start S
%skip /[ \t\r\n]+/
%%
S: A 'b' { ++actions; $$ = $1; };
A: 'a' 'b' { ++actions; $$ = $1; }
 | 'a' { ++actions; $$ = $1; };
%%
int main(int argc, char **argv)
{
    assert(argc == 2);
    AUResult r = parse_S(argv[1], strlen(argv[1]), NULL);
    printf("%d %u %zu\n", (int)r.status, actions, r.offset);
    au_result_free(&r);
    return 0;
}
