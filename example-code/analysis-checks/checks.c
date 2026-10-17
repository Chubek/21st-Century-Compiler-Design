/* Concrete/abstract cross-checks for Chapters 42--44. C11, standard library.
   See README.md for the build command. Keep assertions enabled.
   This fixture's small finite values cannot overflow int. */
#include <assert.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>

enum { DIM = 3, INF = 1000000 };
typedef struct { int d[DIM][DIM]; bool bottom; } DBM;

static int minimum(int a, int b) { return a < b ? a : b; }
static int maximum(int a, int b) { return a > b ? a : b; }
static uint64_t value(int n) { assert(n >= 0 && n < 64); return UINT64_C(1) << n; }

static DBM top(void)
{
    DBM a = {{{0}}, false};
    for (int i = 0; i < DIM; ++i)
        for (int j = 0; j < DIM; ++j)
            a.d[i][j] = i == j ? 0 : INF;
    return a;
}

static DBM close_dbm(DBM a)
{
    if (a.bottom) return a;
    for (int i = 0; i < DIM; ++i) a.d[i][i] = minimum(a.d[i][i], 0);
    for (int k = 0; k < DIM; ++k) {
        for (int i = 0; i < DIM; ++i)
            for (int j = 0; j < DIM; ++j)
                if (a.d[i][k] != INF && a.d[k][j] != INF)
                    a.d[i][j] = minimum(a.d[i][j], a.d[i][k] + a.d[k][j]);
        for (int i = 0; i < DIM; ++i)
            if (a.d[i][i] < 0) { a.bottom = true; return a; }
    }
    return a;
}

static DBM relation(int upper, int distance)
{
    DBM a = top();
    a.d[0][1] = 0;
    a.d[1][0] = upper;
    a.d[2][1] = distance;
    a.d[1][2] = -distance;
    return close_dbm(a);
}

static bool accepts(DBM a, int x, int y)
{
    if (a.bottom) return false;
    const int v[DIM] = {0, x, y};
    for (int i = 0; i < DIM; ++i)
        for (int j = 0; j < DIM; ++j)
            if (v[i] - v[j] > a.d[i][j]) return false;
    return true;
}

/* Independent oracle: maximize all differences over a concrete finite set. */
static DBM concrete_hull(const int pairs[][2], int count)
{
    DBM a = top();
    assert(count > 0);
    for (int i = 0; i < DIM; ++i)
        for (int j = 0; j < DIM; ++j) a.d[i][j] = -INF;
    for (int p = 0; p < count; ++p) {
        const int v[DIM] = {0, pairs[p][0], pairs[p][1]};
        for (int i = 0; i < DIM; ++i)
            for (int j = 0; j < DIM; ++j)
                a.d[i][j] = maximum(a.d[i][j], v[i] - v[j]);
    }
    return a;
}

static void equal_dbm(DBM a, DBM b)
{
    assert(a.bottom == b.bottom);
    if (a.bottom) return;
    for (int i = 0; i < DIM; ++i)
        for (int j = 0; j < DIM; ++j) assert(a.d[i][j] == b.d[i][j]);
}

static DBM join_dbm(DBM a, DBM b)
{
    if (a.bottom) return b;
    if (b.bottom) return a;
    for (int i = 0; i < DIM; ++i)
        for (int j = 0; j < DIM; ++j)
            a.d[i][j] = maximum(a.d[i][j], b.d[i][j]);
    return a;
}

static DBM header(DBM a, int bound, int distance)
{
    DBM entry = relation(0, distance);
    a.d[1][0] = minimum(a.d[1][0], bound - 1);
    a = close_dbm(a);
    if (a.bottom) return entry;
    /* Simultaneous translation of both counters by one. */
    const int delta[DIM] = {0, 1, 1};
    for (int i = 0; i < DIM; ++i)
        for (int j = 0; j < DIM; ++j)
            if (a.d[i][j] != INF) a.d[i][j] += delta[i] - delta[j];
    return join_dbm(entry, a);
}

static void square_check(int left, int right, int expected_low, int expected_high)
{
    int low = INF, high = -INF;
    for (int x = left; x <= right; ++x) {
        low = minimum(low, x * x);
        high = maximum(high, x * x);
    }
    assert(low == expected_low && high == expected_high);
    assert(minimum(left * left, right * right) > low);
}

static void soundness_examples(void)
{
    uint64_t y = 0, z = 0;
    const int branches[2] = {-4, 6};
    for (int b = 0; b < 2; ++b) {
        y |= value(branches[b] + 5);
        z |= value(branches[b] * branches[b]);
    }
    assert(y == (value(1) | value(11)));
    assert(z == (value(16) | value(36)));
    square_check(-4, 6, 0, 36);
    square_check(-3, 8, 0, 64);
    int lo = INF, hi = -INF;
    for (int x = 250; x <= 255; ++x) {
        int r = (x + 1) % 256;
        lo = minimum(lo, r); hi = maximum(hi, r);
    }
    assert(lo == 0 && hi == 255);
    const int wrap_expected[3] = {255, 0, 1};
    for (int x = 253; x <= 255; ++x)
        assert((x + 2) % 256 == wrap_expected[x - 253]);
    assert(2 + 3 > 4); /* Candidate [2,4] fails the transfer inclusion. */
    /* Reverse-dependency traversal for exercise 42.4: a,b,c,d,e = 0..4. */
    const unsigned readers[5] = {10U, 4U, 0U, 0U, 8U};
    unsigned seen = 2U, pending = 2U;
    while (pending) {
        for (unsigned u = 0; u < 5; ++u) {
            if (!(pending & (1U << u))) continue;
            pending &= ~(1U << u);
            unsigned fresh = readers[u] & ~seen;
            seen |= fresh; pending |= fresh;
        }
    }
    assert(seen == 6U);
}

static void relation_examples(void)
{
    const int six[][2] = {{0,3},{1,4},{2,5},{3,6},{4,7},{5,8}};
    const int four[][2] = {{0,3},{1,4},{2,5},{3,6}};
    DBM a = relation(5, 3);
    equal_dbm(a, concrete_hull(six, 6));
    assert(!accepts(a, 5, 3));
    a.d[2][0] = 6;
    a = close_dbm(a);
    equal_dbm(a, concrete_hull(four, 4));
    a.d[0][1] = -4;
    assert(close_dbm(a).bottom);

    const int exercise[][2] = {{0,2},{1,3},{2,4},{3,5},{4,6}};
    DBM e = relation(4, 2);
    equal_dbm(e, concrete_hull(exercise, 5));
    e.d[2][0] = 4;
    e = close_dbm(e);
    equal_dbm(e, relation(2, 2));
    e.d[0][1] = -3;
    assert(close_dbm(e).bottom);

    const int paths[][2] = {{1,7},{7,1}};
    DBM merged = concrete_hull(paths, 2);
    assert(accepts(merged, 1, 1) && accepts(merged, 7, 7));
    int low = INF, high = -INF;
    for (int x = 1; x <= 7; ++x)
        for (int y = 1; y <= 7; ++y)
            if (accepts(merged, x, y)) {
                low = minimum(low, x + y); high = maximum(high, x + y);
            }
    assert(low == 2 && high == 14);
    for (int p = 0; p < 2; ++p) assert(paths[p][0] + paths[p][1] == 8);

    const int exercise_paths[][2] = {{2,9},{9,2}};
    DBM exercise_merge = concrete_hull(exercise_paths, 2);
    assert(accepts(exercise_merge, 2, 2) && accepts(exercise_merge, 9, 9));
    for (int p = 0; p < 2; ++p)
        assert(exercise_paths[p][0] + exercise_paths[p][1] == 11);
}

static void loop_example(int bound, int distance)
{
    DBM w = relation(0, distance);
    unsigned evaluations = 0;
    for (;;) {
        DBM c = header(close_dbm(w), bound, distance);
        bool changed = false;
        ++evaluations;
        for (int i = 0; i < DIM; ++i)
            for (int j = 0; j < DIM; ++j)
                if (c.d[i][j] > w.d[i][j]) {
                    w.d[i][j] = INF;
                    changed = true;
                }
        if (!changed) break;
        assert(evaluations < 10);
    }
    assert(evaluations == 2);
    assert(w.d[1][0] == INF && w.d[2][0] == INF);
    assert(w.d[2][1] == distance && w.d[1][2] == -distance);
    DBM narrowed = header(close_dbm(w), bound, distance);
    equal_dbm(narrowed, relation(bound, distance));
    equal_dbm(header(narrowed, bound, distance), narrowed);
    /* Enumerate the entire concrete execution, including the exit header. */
    int i = 0, j = distance;
    for (;;) {
        assert(accepts(narrowed, i, j));
        if (i >= bound) break;
        ++i; ++j;
    }
    assert(i == bound && j == bound + distance);
    narrowed.d[0][1] = -bound;
    narrowed = close_dbm(narrowed);
    assert(narrowed.d[1][0] == bound && narrowed.d[0][1] == -bound);
    assert(narrowed.d[2][0] == bound + distance);
    assert(narrowed.d[0][2] == -(bound + distance));
}

static int concrete_recursive(int n)
{
    return n == 0 ? 2 : 1 + concrete_recursive(n - 1);
}

static void summary_example(int bound)
{
    bool summary[5][7] = {{false}};
    int round = 0;
    for (;;) {
        bool next[5][7] = {{false}};
        int added = 0;
        ++round;
        for (int n = 0; n <= bound; ++n)
            for (int result = 0; result < 7; ++result) {
                bool base = n == 0 && result == 2;
                bool recursive = n > 0 && result > 0 && summary[n - 1][result - 1];
                next[n][result] = summary[n][result] || base || recursive;
                added += next[n][result] && !summary[n][result];
            }
        if (!added) break;
        assert(added == 1 && round <= bound + 1);
        assert(next[round - 1][round + 1]);
        for (int n = 0; n <= bound; ++n)
            for (int result = 0; result < 7; ++result)
                summary[n][result] = next[n][result];
    }
    assert(round == bound + 2);
    for (int n = 0; n <= bound; ++n)
        for (int result = 0; result < 7; ++result)
            assert(summary[n][result] == (result == concrete_recursive(n)));
}

static void heap_examples(void)
{
    int objects[2] = {0, 0};
    objects[0] = 7;
    uint64_t site = value(objects[0]) | value(objects[1]);
    assert(site == (value(0) | value(7)) && site != value(7));
    int unique = 2;
    unique = 5;
    assert(value(unique) == value(5));
    uint64_t u = 0, v = 0;
    for (int target = 0; target < 2; ++target) {
        int cells[2] = {2, 9};
        cells[target] = 5;
        u |= value(cells[0]); v |= value(cells[1]);
    }
    assert(u == (value(2) | value(5)));
    assert(v == (value(5) | value(9)));
    int three[3] = {1, 1, 1};
    three[1] = 6;
    assert((value(three[0]) | value(three[1]) | value(three[2])) ==
           (value(1) | value(6)));
    uint64_t results = 0;
    for (int q = 0; q < 2; ++q) {
        int cells[2] = {2, 9};
        cells[0] = 5;
        results |= value(cells[q]);
        assert(cells[q] == (q == 0 ? 5 : 9));
    }
    assert(results == (value(5) | value(9)));
    assert(4 + 2 == 6 && 9 + 2 == 11 && 1 + 2 == 3 && 8 + 2 == 10);
}

int main(void)
{
    soundness_examples();
    relation_examples();
    loop_example(6, 3);
    loop_example(7, 4);
    summary_example(3);
    summary_example(4);
    heap_examples();
    puts("Chapters 42--44: all concrete/abstract checks passed.");
    return 0;
}
