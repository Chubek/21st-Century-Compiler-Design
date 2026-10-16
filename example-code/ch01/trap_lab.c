/* Chapter 1: traps, effects, and fuel in isolation. Requires C11.
   The extended tree below is a laboratory specimen. It does not change
   expr.h, expr.c, stack.c, or their contracts. */
#include <assert.h>
#include <stdbool.h>
#include <stdio.h>

typedef enum { TLIT, TINPUT, TADD, TMUL, TDIV } TKind;

typedef struct TExpr {
    TKind kind;
    unsigned literal;
    struct TExpr *left;
    struct TExpr *right;
} TExpr;

/* Evaluate e at input x. Return false on a division-by-zero trap.
   On success, store the value in V and return true. */
static bool teval(const TExpr *e, unsigned x, unsigned *out)
{
    unsigned a, b;
    assert(e != 0 && out != 0 && x <= 255U);
    if (e->kind == TLIT) {
        assert(e->literal <= 255U);
        *out = e->literal;
        return true;
    }
    if (e->kind == TINPUT) {
        *out = x;
        return true;
    }
    assert(e->kind == TADD || e->kind == TMUL || e->kind == TDIV);
    assert(e->left != 0 && e->right != 0);
    if (!teval(e->left, x, &a))
        return false;
    if (!teval(e->right, x, &b))
        return false;
    if (e->kind == TADD)
        *out = (a + b) % 256U;
    else if (e->kind == TMUL)
        *out = (a * b) % 256U;
    else {
        if (b == 0U)
            return false;
        *out = a / b;
    }
    return true;
}

/* Fold constant children. A division folds only when its divisor is a
   nonzero literal, so a trapping division is never replaced by a value. */
static void tfold(TExpr *e)
{
    unsigned value;
    assert(e != 0);
    if (e->kind == TLIT || e->kind == TINPUT)
        return;
    assert(e->kind == TADD || e->kind == TMUL || e->kind == TDIV);
    tfold(e->left);
    tfold(e->right);
    if (e->left->kind != TLIT || e->right->kind != TLIT)
        return;
    if (e->kind == TDIV && e->right->literal == 0U)
        return;
    assert(teval(e, 0U, &value));
    e->kind = TLIT;
    e->literal = value;
    e->left = 0;
    e->right = 0;
}

typedef struct {
    unsigned tags[8];
    unsigned count;
} EventLog;

/* Record tag in order and return value unchanged. */
static unsigned logged(EventLog *log, unsigned tag, unsigned value)
{
    assert(log != 0 && log->count < 8U);
    log->tags[log->count++] = tag;
    return value;
}

/* A loop that needs exactly `needed' steps. With less fuel it reports
   failure instead of diverging; this keeps the laboratory terminating. */
static bool run_with_fuel(unsigned needed, unsigned fuel, unsigned *done)
{
    unsigned i;
    assert(done != 0);
    for (i = 0U; i < needed; ++i) {
        if (i >= fuel)
            return false;
    }
    *done = needed;
    return true;
}

static void test_division_pairs(void)
{
    for (unsigned a = 0U; a < 256U; ++a) {
        for (unsigned b = 0U; b < 256U; ++b) {
            TExpr left = {TLIT, a, 0, 0};
            TExpr right = {TLIT, b, 0, 0};
            TExpr root = {TDIV, 0U, &left, &right};
            unsigned out = 999U;
            bool ok = teval(&root, 0U, &out);
            if (b == 0U) {
                assert(!ok);
            } else {
                assert(ok && out == a / b);
                assert(out <= 255U);
            }
            tfold(&root);
            if (b == 0U) {
                assert(root.kind == TDIV);
            } else {
                assert(root.kind == TLIT && root.literal == a / b);
            }
        }
    }
}

static void test_zero_divisor_folding(void)
{
    /* (249 + 7) wraps to 0, so the divisor folds to zero and must stay. */
    TExpr x = {TINPUT, 0U, 0, 0};
    TExpr seven = {TLIT, 7U, 0, 0};
    TExpr two49 = {TLIT, 249U, 0, 0};
    TExpr num = {TADD, 0U, &x, &seven};
    TExpr den = {TADD, 0U, &two49, &seven};
    TExpr root = {TDIV, 0U, &num, &den};
    unsigned before_trap = 0U;
    for (unsigned i = 0U; i < 256U; ++i) {
        unsigned out = 999U;
        assert(!teval(&root, i, &out));
        before_trap += 1U;
    }
    assert(before_trap == 256U);
    tfold(&root);
    assert(root.kind == TDIV);
    assert(root.right->kind == TLIT && root.right->literal == 0U);
    for (unsigned i = 0U; i < 256U; ++i) {
        unsigned out = 999U;
        assert(!teval(&root, i, &out));
    }
    /* (x + 7) / (3 + 5) folds its divisor to 8 and traps for no input. */
    TExpr three = {TLIT, 3U, 0, 0};
    TExpr five = {TLIT, 5U, 0, 0};
    TExpr xn = {TINPUT, 0U, 0, 0};
    TExpr s7 = {TLIT, 7U, 0, 0};
    TExpr top = {TADD, 0U, &xn, &s7};
    TExpr bot = {TADD, 0U, &three, &five};
    TExpr safe = {TDIV, 0U, &top, &bot};
    tfold(&safe);
    assert(safe.kind == TDIV);
    assert(safe.right->kind == TLIT && safe.right->literal == 8U);
    for (unsigned i = 0U; i < 256U; ++i) {
        unsigned out = 999U, expected;
        assert(teval(&safe, i, &out));
        expected = ((i + 7U) % 256U) / 8U;
        assert(out == expected);
    }
    assert(((9U + 7U) % 256U) / 8U == 2U);
    assert(((250U + 7U) % 256U) / 8U == 0U);
}

static void test_effect_order(void)
{
    EventLog left_first = {{0}, 0U};
    EventLog right_first = {{0}, 0U};
    unsigned l1 = logged(&left_first, 1U, 10U);
    unsigned l2 = logged(&left_first, 2U, 20U);
    unsigned r2 = logged(&right_first, 2U, 20U);
    unsigned r1 = logged(&right_first, 1U, 10U);
    unsigned a = l1 + l2;
    unsigned b = r1 + r2;
    assert(a == 30U && b == 30U);
    assert(left_first.count == 2U && right_first.count == 2U);
    assert(left_first.tags[0] == 1U && left_first.tags[1] == 2U);
    assert(right_first.tags[0] == 2U && right_first.tags[1] == 1U);
}

static void test_fuel(void)
{
    unsigned done = 999U;
    assert(!run_with_fuel(5U, 3U, &done) && done == 999U);
    assert(run_with_fuel(5U, 5U, &done) && done == 5U);
    assert(run_with_fuel(0U, 0U, &done) && done == 0U);
}

int main(void)
{
    test_division_pairs();
    test_zero_divisor_folding();
    test_effect_order();
    test_fuel();
    puts("trap lab: guards, order, and fuel checks passed");
    return 0;
}
