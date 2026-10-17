/* Recompute Chapters 45--46 examples and solutions. C11, standard library.
   cc -std=c11 -Wall -Wextra -Wpedantic -Werror checks.c -o checks
   Keep assertions enabled. Tiny bit sets favor inspectable calculations. */
#include <assert.h>
#include <stdbool.h>
#include <stdio.h>

typedef unsigned Set;
typedef struct { unsigned n; Set edge[8]; } Graph;
#define BIT(i) (1U << (i))

static Set universe(const Graph *g) { return BIT(g->n) - 1U; }

static Set pre(const Graph *g, Set x)
{
    Set r = 0;
    for (unsigned s = 0; s < g->n; ++s)
        if (g->edge[s] & x) r |= BIT(s);
    return r;
}

static Set post(const Graph *g, Set x)
{
    Set r = 0;
    for (unsigned s = 0; s < g->n; ++s)
        if (x & BIT(s)) r |= g->edge[s];
    return r;
}

static Set eu(const Graph *g, Set p, Set q)
{
    Set x = q;
    for (;;) {
        Set y = q | (p & pre(g, x));
        if (x == y) return x;
        x = y;
    }
}

static Set eg(const Graph *g, Set p)
{
    Set x = p;
    for (;;) {
        Set y = p & pre(g, x);
        if (x == y) return x;
        x = y;
    }
}

static Set af(const Graph *g, Set q)
{
    return universe(g) ^ eg(g, universe(g) ^ q);
}

static Set ag(const Graph *g, Set p)
{
    return universe(g) ^ eu(g, universe(g), universe(g) ^ p);
}

/* Independent finite-path search: a shortest until witness is simple. */
static bool until_path(const Graph *g, unsigned s, Set p, Set q, unsigned left)
{
    if (q & BIT(s)) return true;
    if (!(p & BIT(s)) || left == 0) return false;
    for (unsigned t = 0; t < g->n; ++t)
        if ((g->edge[s] & BIT(t)) && until_path(g, t, p, q, left - 1))
            return true;
    return false;
}

/* A path of n edges within P repeats a state and supplies an infinite path. */
static bool long_path(const Graph *g, unsigned s, Set p, unsigned left)
{
    if (!(p & BIT(s))) return false;
    if (left == 0) return true;
    for (unsigned t = 0; t < g->n; ++t)
        if ((g->edge[s] & BIT(t)) && long_path(g, t, p, left - 1))
            return true;
    return false;
}

static void distances(const Graph *g, Set initial, int d[8])
{
    unsigned queue[8], head = 0, tail = 0;
    for (unsigned s = 0; s < g->n; ++s) {
        d[s] = (initial & BIT(s)) ? 0 : -1;
        if (d[s] == 0) queue[tail++] = s;
    }
    while (head < tail) {
        unsigned s = queue[head++];
        for (unsigned t = 0; t < g->n; ++t) {
            if ((g->edge[s] & BIT(t)) && d[t] == -1) {
                d[t] = d[s] + 1;
                queue[tail++] = t;
            }
        }
    }
}

enum Monitor { CLEAR, PENDING, BAD };
enum Event { NONE, FAIL, REBUILD, COMMIT };

static enum Monitor step(enum Monitor q, enum Event e)
{
    if (q == BAD) return BAD;
    if (e == FAIL) return PENDING;
    if (e == REBUILD) return CLEAR;
    if (e == COMMIT && q == PENDING) return BAD;
    return q;
}

static void check_ch45(void)
{
    Graph cache = {7, {BIT(1), BIT(1)|BIT(2), BIT(3), BIT(3)|BIT(4),
                       BIT(4)|BIT(5), BIT(5), BIT(6)}};
    Set resident = BIT(1)|BIT(2)|BIT(3)|BIT(4);
    Set metadata = BIT(2)|BIT(3)|BIT(4);
    Set pinned = BIT(3)|BIT(4)|BIT(6), published = BIT(3);
    Set reachable = eu(&cache, universe(&cache), pinned & ~resident);
    assert(!(reachable & BIT(0)));
    assert((published & ~metadata) == 0);
    cache.edge[4] |= BIT(6);
    int d[8];
    distances(&cache, BIT(0), d);
    assert(d[6] == 5);
    assert(eu(&cache, universe(&cache), pinned & ~resident) & BIT(0));

    /* Totalization of u -> h and u -> z gives loops at h and z. */
    Graph terminal = {3, {BIT(1)|BIT(2), BIT(1), BIT(2)}};
    assert(eu(&terminal, 7, BIT(1)) == (BIT(0)|BIT(1)));
    assert(af(&terminal, BIT(1)) == BIT(1));
    assert(ag(&terminal, 7 ^ BIT(2)) == BIT(1));

    Graph pass = {4, {BIT(1), BIT(1)|BIT(2), BIT(3), BIT(3)}};
    Set done = BIT(3), valid = BIT(2)|BIT(3);
    assert(eu(&pass, 15, done) == 15);
    assert(af(&pass, done) == valid);
    assert(ag(&pass, eu(&pass, 15, done)) == 15);
    assert(eg(&pass, 15 ^ done) == (BIT(0)|BIT(1)));
    assert((15 ^ pre(&pass, 15 ^ valid)) == valid);
    pass.n = 5;
    pass.edge[1] |= BIT(4);
    pass.edge[4] = BIT(4);
    assert(eu(&pass, 31, done) == 15);
    assert(ag(&pass, eu(&pass, 31, done)) == valid);

    assert(step(step(step(CLEAR, FAIL), NONE), COMMIT) == BAD);
    assert(step(step(step(CLEAR, FAIL), REBUILD), COMMIT) == CLEAR);
    enum Monitor q = CLEAR;
    const enum Event events[] = {FAIL, REBUILD, FAIL, COMMIT, REBUILD};
    const enum Monitor expected[] = {PENDING, CLEAR, PENDING, BAD, BAD};
    for (unsigned i = 0; i < 5; ++i) {
        q = step(q, events[i]);
        assert(q == expected[i]);
    }

    /* For a repeated finite loop, FG enabled means every loop state enables;
       GF enabled/taken means some loop position enables/takes the action. */
    bool enabled_all = false, enabled_some = true, taken_some = false;
    assert(!enabled_all || taken_some);       /* weak fairness */
    assert(!(!enabled_some || taken_some));   /* not strong fairness */
    enabled_all = true;
    assert(!(!enabled_all || taken_some));    /* now not weakly fair */
}

static void check_ch46(void)
{
    Graph job = {6, {BIT(1)|BIT(4), BIT(1)|BIT(2), BIT(3), BIT(3),
                     BIT(5), BIT(5)}};
    int d[8];
    distances(&job, BIT(0), d);
    const int expected[] = {0, 1, 2, 3, 1, 2};
    for (unsigned s = 0; s < job.n; ++s) assert(d[s] == expected[s]);
    distances(&job, BIT(0)|BIT(4), d);
    assert(d[5] == 1);

    Set p = BIT(0)|BIT(1)|BIT(2)|BIT(4), x = BIT(3), y = p;
    const Set xs[] = {8, 12, 14, 15, 15};
    const Set ys[] = {23, 3, 3, 3, 3};
    for (unsigned i = 0; i < 5; ++i) {
        assert(x == xs[i] && y == ys[i]);
        x = BIT(3) | (p & pre(&job, x));
        y = p & pre(&job, y);
    }
    assert(eu(&job, BIT(0)|BIT(1), BIT(2)|BIT(3)) == 15);
    assert(eg(&job, 63 ^ BIT(5)) == 15);
    assert(af(&job, BIT(3)) == (BIT(2)|BIT(3)));

    /* Exhaust all 64 possible operand sets, not just the displayed cases. */
    for (p = 0; p < 64; ++p) {
        Set always = eg(&job, p);
        for (unsigned s = 0; s < job.n; ++s)
            assert(((always & BIT(s)) != 0) == long_path(&job, s, p, job.n));
        for (Set q = 0; q < 64; ++q) {
            Set until = eu(&job, p, q);
            for (unsigned s = 0; s < job.n; ++s)
                assert(((until & BIT(s)) != 0) ==
                       until_path(&job, s, p, q, job.n - 1));
        }
    }
    job.edge[1] &= ~BIT(1);
    assert(af(&job, BIT(3)) == (BIT(1)|BIT(2)|BIT(3)));

    Graph chain = {5, {BIT(1), BIT(2), BIT(3), BIT(4), BIT(4)}};
    for (unsigned k = 0; k < 8; ++k)
        assert(until_path(&chain, 0, 31, BIT(4), k) == (k >= 4));

    Graph acceptance = {3, {BIT(1)|BIT(2), BIT(0), BIT(0)}};
    const unsigned walk[] = {0, 1, 0, 2, 0};
    Set visited = 0;
    for (unsigned i = 0; i < 4; ++i) {
        assert(acceptance.edge[walk[i]] & BIT(walk[i + 1]));
        visited |= BIT(walk[i]);
    }
    assert((visited & BIT(1)) && (visited & BIT(2)));

    /* Concrete order: i,u,d,w,e; abstract A merges u and w. */
    Graph concrete = {5, {BIT(1), BIT(2), BIT(2), BIT(4), BIT(4)}};
    Set a = BIT(1)|BIT(3), endpoints = BIT(0);
    endpoints = post(&concrete, endpoints) & a;
    assert(endpoints == BIT(1));
    assert((post(&concrete, endpoints) & BIT(4)) == 0);
    assert((post(&concrete, endpoints) & BIT(2)) == BIT(2));
    assert(!(eu(&concrete, 31, BIT(4)) & BIT(0)));
    concrete.edge[0] |= BIT(3);
    endpoints = post(&concrete, BIT(0)) & a;
    assert(endpoints == a);
    assert((post(&concrete, endpoints) & BIT(4)) == BIT(4));
}

int main(void)
{
    check_ch45();
    check_ch46();
    puts("Chapters 45--46: tables, witnesses, fixed points, and solutions pass.");
    return 0;
}
