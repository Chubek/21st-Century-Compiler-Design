/* Chapter 1: translation, verifier, and composition tests. C11. */
#include "stack.h"
#include <assert.h>
#include <stdio.h>

static void compare_all(Expr *e)
{
    Code original = {0}, optimized = {0};
    size_t peak = 0;
    unsigned expected[256];
    assert(compile_expr(e, &original, CODE_CAP));
    assert(verify_code(&original, &peak) && peak > 0);
    for (unsigned x = 0; x < 256U; ++x) {
        unsigned result = 999U;
        expected[x] = eval(e, x);
        assert(run_code(&original, x, &result));
        assert(result == expected[x]);
    }
    fold(e);
    assert(compile_expr(e, &optimized, CODE_CAP));
    assert(optimized.used <= original.used);
    for (unsigned x = 0; x < 256U; ++x) {
        unsigned result = 999U;
        assert(run_code(&optimized, x, &result));
        assert(result == expected[x]);
    }
}

static Expr *generate(Expr arena[31], size_t *used,
                      unsigned depth, unsigned *seed)
{
    assert(*used < 31);
    Expr *e = &arena[(*used)++];
    *seed = (251U * *seed + 17U) % 256U;
    unsigned choice = *seed % 4U;
    e->literal = *seed;
    e->left = e->right = 0;
    if (depth == 0 || choice < 2U) {
        e->kind = choice == 0U ? INPUT : LITERAL;
    } else {
        e->kind = choice == 2U ? ADD : MUL;
        e->left = generate(arena, used, depth - 1U, seed);
        e->right = generate(arena, used, depth - 1U, seed);
    }
    return e;
}

static void test_worked_trace(void)
{
    Expr x = {INPUT, 0U, 0, 0}, seven = {LITERAL, 7U, 0, 0};
    Expr three = {LITERAL, 3U, 0, 0}, five = {LITERAL, 5U, 0, 0};
    Expr left = {ADD, 0U, &x, &seven};
    Expr right = {ADD, 0U, &three, &five};
    Expr root = {MUL, 0U, &left, &right};
    Code code = {0};
    size_t peak = 0;
    assert(compile_expr(&root, &code, 7));
    const Opcode ops[] = {GET_INPUT, PUSH, ADD8, PUSH, PUSH, ADD8, MUL8};
    const unsigned args[] = {0U, 7U, 0U, 3U, 5U, 0U, 0U};
    assert(code.used == 7);
    for (size_t i = 0; i < 7; ++i) {
        assert(code.items[i].op == ops[i]);
        assert(code.items[i].arg == args[i]);
    }
    assert(verify_code(&code, &peak) && peak == 3);
    compare_all(&root);
    Code folded = {0};
    assert(compile_expr(&root, &folded, 5));
    assert(folded.used == 5);
    assert(verify_code(&folded, &peak) && peak == 2);
}

static void reject(Code code)
{
    unsigned result = 999U;
    size_t peak = 999;
    assert(!verify_code(&code, &peak));
    assert(peak == 999);
    assert(!run_code(&code, 0U, &result));
    assert(result == 999U);
}

static void test_rejection(void)
{
    Code code = {0};
    reject(code);
    code.used = 1;
    code.items[0] = (Instruction){ADD8, 0U};
    reject(code);
    code.items[0] = (Instruction){PUSH, 256U};
    reject(code);
    code.items[0] = (Instruction){GET_INPUT, 1U};
    reject(code);
    code.items[0] = (Instruction){(Opcode)99, 0U};
    reject(code);
    code.items[0] = (Instruction){PUSH, 0U};
    code.items[1] = (Instruction){PUSH, 1U};
    code.used = 2;
    reject(code);
    code.items[2] = (Instruction){MUL8, 1U};
    code.used = 3;
    reject(code);
    code.used = CODE_CAP + 1;
    reject(code);
    code.used = 1;
    unsigned result = 999U;
    assert(!run_code(&code, 256U, &result) && result == 999U);
}

static void test_capacity_and_prefix(void)
{
    Expr x = {INPUT, 0U, 0, 0};
    Expr seven = {LITERAL, 7U, 0, 0};
    Expr root = {ADD, 0U, &x, &seven};
    for (size_t limit = 0; limit < 3; ++limit) {
        Code code = {0};
        assert(!compile_expr(&root, &code, limit));
        assert(code.used == 0);
    }
    Code code = {0};
    code.items[code.used++] = (Instruction){PUSH, 23U};
    assert(!compile_expr(&root, &code, 3));
    assert(code.used == 1 && code.items[0].arg == 23U);
    assert(compile_expr(&root, &code, 4));
    assert(code.used == 4 && code.items[0].arg == 23U);
    /* A valid expression fragment appended to a value is not a whole program. */
    size_t peak;
    assert(!verify_code(&code, &peak));
}

static void test_literal_pairs(void)
{
    for (unsigned a = 0; a < 256U; ++a)
        for (unsigned b = 0; b < 256U; ++b)
            for (unsigned op = 0; op < 2U; ++op) {
                Expr left = {LITERAL, a, 0, 0};
                Expr right = {LITERAL, b, 0, 0};
                Expr root = {op == 0U ? ADD : MUL, 0U, &left, &right};
                Code code = {0};
                unsigned result;
                assert(compile_expr(&root, &code, 3));
                assert(run_code(&code, 0U, &result));
                assert(result == (op == 0U ? (a + b) % 256U
                                           : (a * b) % 256U));
            }
}

int main(void)
{
    test_worked_trace();
    test_rejection();
    test_capacity_and_prefix();
    test_literal_pairs();
    for (unsigned initial = 0; initial < 256U; ++initial) {
        Expr arena[31];
        size_t used = 0;
        unsigned seed = initial;
        compare_all(generate(arena, &used, 4U, &seed));
    }
    puts("chapter 1 stack translation: all checks passed");
    return 0;
}
