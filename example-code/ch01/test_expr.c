/* Chapter 1: finite-domain checks and generated-tree tests. C11. */
#include "expr.h"
#include <assert.h>
#include <stdio.h>

static void test_binary_operations(void)
{
    for (unsigned a = 0; a < 256U; ++a) {
        for (unsigned b = 0; b < 256U; ++b) {
            for (unsigned op = 0; op < 2U; ++op) {
                Expr left = {LITERAL, a, 0, 0};
                Expr right = {LITERAL, b, 0, 0};
                Expr root = {op == 0U ? ADD : MUL, 0U,
                             &left, &right};
                unsigned expected = op == 0U
                    ? (a + b) % 256U : (a * b) % 256U;
                assert(eval(&root, 0U) == expected);
                fold(&root);
                assert(root.kind == LITERAL);
                assert(root.literal == expected);
                assert(root.left == 0 && root.right == 0);
            }
        }
    }
}

static void test_worked_example(void)
{
    Expr x = {INPUT, 0U, 0, 0};
    Expr seven = {LITERAL, 7U, 0, 0};
    Expr three = {LITERAL, 3U, 0, 0};
    Expr five = {LITERAL, 5U, 0, 0};
    Expr left = {ADD, 0U, &x, &seven};
    Expr right = {ADD, 0U, &three, &five};
    Expr root = {MUL, 0U, &left, &right};
    unsigned before[256];
    for (unsigned i = 0; i < 256U; ++i) {
        before[i] = eval(&root, i);
        assert(before[i] == (((i + 7U) % 256U) * 8U) % 256U);
    }
    fold(&root);
    assert(root.kind == MUL && left.kind == ADD);
    assert(right.kind == LITERAL && right.literal == 8U);
    for (unsigned i = 0; i < 256U; ++i)
        assert(eval(&root, i) == before[i]);
    fold(&root);
    for (unsigned i = 0; i < 256U; ++i)
        assert(eval(&root, i) == before[i]);
}

/* A fixed generator makes every failure reproducible. */
static unsigned next_seed(unsigned *seed)
{
    *seed = (251U * *seed + 17U) % 256U;
    return *seed;
}

static Expr *generate(Expr arena[31], unsigned *used,
                      unsigned depth, unsigned *seed)
{
    assert(*used < 31U);
    Expr *node = &arena[(*used)++];
    unsigned choice = next_seed(seed) % 4U;
    if (depth == 0U || choice < 2U) {
        node->kind = choice == 0U ? INPUT : LITERAL;
        node->literal = next_seed(seed);
        node->left = 0;
        node->right = 0;
    } else {
        node->kind = choice == 2U ? ADD : MUL;
        node->literal = 0U;
        node->left = generate(arena, used, depth - 1U, seed);
        node->right = generate(arena, used, depth - 1U, seed);
    }
    return node;
}

static void test_generated_trees(void)
{
    for (unsigned initial = 0; initial < 256U; ++initial) {
        Expr arena[31];
        unsigned used = 0U, seed = initial;
        Expr *root = generate(arena, &used, 4U, &seed);
        unsigned before[256];
        for (unsigned x = 0; x < 256U; ++x)
            before[x] = eval(root, x);
        fold(root);
        for (unsigned x = 0; x < 256U; ++x)
            assert(eval(root, x) == before[x]);
    }
}

static void test_exercises(void)
{
    Expr fourteen = {LITERAL, 14U, 0, 0};
    Expr nineteen = {LITERAL, 19U, 0, 0};
    Expr six = {LITERAL, 6U, 0, 0};
    Expr five = {LITERAL, 5U, 0, 0};
    Expr left = {ADD, 0U, &fourteen, &nineteen};
    Expr right = {ADD, 0U, &six, &five};
    Expr root = {MUL, 0U, &left, &right};
    fold(&root);
    assert(root.kind == LITERAL && root.literal == 107U);
    for (unsigned x = 0; x < 256U; ++x) {
        assert((x * 0U) % 256U == 0U);
        assert((x + 0U) % 256U == x);
    }
    assert((250U + 10U) % 256U == 4U);
}

/* Check the root replacement suggested by the add-zero exercise. */
static void test_add_zero_replacement(void)
{
    for (unsigned variant = 0; variant < 2U; ++variant) {
        Expr x = {INPUT, 0U, 0, 0};
        Expr seven = {LITERAL, 7U, 0, 0};
        Expr zero = {LITERAL, 0U, 0, 0};
        Expr product = {MUL, 0U, &x, &seven};
        Expr root = {ADD, 0U, variant == 0U ? &x : &product,
                     &zero};
        unsigned before[256];
        for (unsigned i = 0; i < 256U; ++i)
            before[i] = eval(&root, i);
        fold(&root);
        assert(root.kind == ADD && root.right->kind == LITERAL);
        assert(root.right->literal == 0U);
        root = *root.left;
        assert(root.kind == (variant == 0U ? INPUT : MUL));
        for (unsigned i = 0; i < 256U; ++i)
            assert(eval(&root, i) == before[i]);
    }
}

int main(void)
{
    test_binary_operations();
    test_worked_example();
    test_generated_trees();
    test_exercises();
    test_add_zero_replacement();
    puts("chapter 1: all checks passed");
    return 0;
}
