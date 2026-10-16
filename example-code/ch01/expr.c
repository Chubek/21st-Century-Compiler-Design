/* Chapter 1: interpreter and constant folder. Requires C11. */
#include "expr.h"
#include <assert.h>

unsigned eval(const Expr *e, unsigned x)
{
    assert(e != 0 && x <= 255U);
    if (e->kind == LITERAL) {
        assert(e->literal <= 255U);
        return e->literal;
    }
    if (e->kind == INPUT)
        return x;
    assert(e->kind == ADD || e->kind == MUL);
    unsigned a = eval(e->left, x);
    unsigned b = eval(e->right, x);
    if (e->kind == ADD)
        return (a + b) % 256U;
    return (a * b) % 256U;
}

void fold(Expr *e)
{
    assert(e != 0);
    if (e->kind == LITERAL || e->kind == INPUT)
        return;
    assert(e->kind == ADD || e->kind == MUL);
    fold(e->left);
    fold(e->right);
    if (e->left->kind != LITERAL || e->right->kind != LITERAL)
        return;
    e->literal = eval(e, 0U);
    e->kind = LITERAL;
    e->left = 0;
    e->right = 0;
}
