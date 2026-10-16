/* Chapter 1: fold (x + 7) * (3 + 5). Requires C11. */
#include "expr.h"
#include <stdio.h>

int main(void)
{
    Expr x = {INPUT, 0U, 0, 0};
    Expr seven = {LITERAL, 7U, 0, 0};
    Expr three = {LITERAL, 3U, 0, 0};
    Expr five = {LITERAL, 5U, 0, 0};
    Expr left = {ADD, 0U, &x, &seven};
    Expr right = {ADD, 0U, &three, &five};
    Expr root = {MUL, 0U, &left, &right};
    const unsigned inputs[] = {0U, 1U, 248U, 249U, 250U, 255U};
    unsigned before[6];

    for (unsigned i = 0; i < 6U; ++i)
        before[i] = eval(&root, inputs[i]);
    fold(&root);
    puts("x before after");
    for (unsigned i = 0; i < 6U; ++i)
        printf("%u %u %u\n", inputs[i], before[i],
               eval(&root, inputs[i]));
    return 0;
}
