/* Chapter 1: isolated wrong-code mutation and greedy reduction. C11.
   Never use mutant_compile as a production compiler. */
#include "stack.h"
#include <assert.h>
#include <stdio.h>

static bool mutant_compile(const Expr *e, Code *code)
{
    if (!compile_expr(e, code, CODE_CAP))
        return false;
    for (size_t i = 0; i < code->used; ++i)
        if (code->items[i].op == MUL8)
            code->items[i].op = ADD8; /* Deliberate wrong-code fault. */
    return true;
}

static bool mismatch(const Expr *e, unsigned x)
{
    Code code = {0};
    unsigned actual;
    size_t peak;
    if (!mutant_compile(e, &code) || !verify_code(&code, &peak))
        return false;
    if (!run_code(&code, x, &actual))
        return false;
    return actual != eval(e, x);
}

typedef struct { unsigned nodes, weight; } Measure;

static Measure measure(const Expr *e)
{
    if (e->kind == LITERAL)
        return (Measure){1U, e->literal};
    if (e->kind == INPUT)
        return (Measure){1U, 256U};
    Measure a = measure(e->left), b = measure(e->right);
    return (Measure){1U + a.nodes + b.nodes, a.weight + b.weight};
}

static bool smaller(Measure a, Measure b)
{
    return a.nodes < b.nodes ||
           (a.nodes == b.nodes && a.weight < b.weight);
}

static bool try_replace(Expr *root, Expr *site, Expr replacement,
                        unsigned x)
{
    Expr saved = *site;
    Measure before = measure(site);
    *site = replacement;
    if (smaller(measure(site), before) && mismatch(root, x))
        return true;
    *site = saved;
    return false;
}

static bool reduce_once(Expr *root, Expr *site, unsigned x)
{
    if (site->kind == ADD || site->kind == MUL) {
        if (try_replace(root, site, *site->left, x))
            return true;
        if (try_replace(root, site, *site->right, x))
            return true;
    }
    for (unsigned n = 0; n <= 1U; ++n) {
        Expr leaf = {LITERAL, n, 0, 0};
        if (try_replace(root, site, leaf, x))
            return true;
    }
    if (site->kind != ADD && site->kind != MUL)
        return false;
    return reduce_once(root, site->left, x) ||
           reduce_once(root, site->right, x);
}

static void report(const Expr *e, unsigned x)
{
    Code code = {0};
    unsigned actual;
    size_t peak;
    assert(mutant_compile(e, &code));
    assert(verify_code(&code, &peak));
    assert(run_code(&code, x, &actual));
    printf("nodes=%u input=%u expected=%u actual=%u\n",
           measure(e).nodes, x, eval(e, x), actual);
}

static void test_accidental_agreement(void)
{
    unsigned total = 0U;
    for (unsigned a = 0; a < 256U; ++a) {
        unsigned matches = 0U;
        for (unsigned b = 0; b < 256U; ++b)
            if ((a * b) % 256U == (a + b) % 256U) {
                assert(a % 2U == 0U && b % 2U == 0U);
                ++matches;
            }
        assert(matches == (a % 2U == 0U ? 1U : 0U));
        total += matches;
    }
    assert(total == 128U);
}

int main(void)
{
    test_accidental_agreement();
    Expr x = {INPUT, 0U, 0, 0}, seven = {LITERAL, 7U, 0, 0};
    Expr three = {LITERAL, 3U, 0, 0}, five = {LITERAL, 5U, 0, 0};
    Expr eleven = {LITERAL, 11U, 0, 0};
    Expr left = {ADD, 0U, &x, &seven};
    Expr right = {ADD, 0U, &three, &five};
    Expr product = {MUL, 0U, &left, &right};
    Expr root = {ADD, 0U, &product, &eleven};
    const unsigned witness = 9U;
    assert(mismatch(&root, witness));
    report(&root, witness);
    unsigned steps = 0;
    while (reduce_once(&root, &root, witness)) {
        assert(++steps < 100U);
        report(&root, witness);
    }
    assert(steps == 5U && root.kind == MUL);
    assert(root.left->kind == LITERAL && root.left->literal == 0U);
    assert(root.right->kind == LITERAL && root.right->literal == 1U);
    for (unsigned a = 0; a < 256U; ++a) {
        Code correct = {0};
        unsigned value;
        assert(mismatch(&root, a));
        assert(compile_expr(&root, &correct, CODE_CAP));
        assert(run_code(&correct, a, &value));
        assert(value == eval(&root, a));
    }
    puts("fault lab: mutant detected; production compiler passes");
    return 0;
}
