/* Chapter 48: C11 independent finite-path and all-pairs-distance oracles.
   Assertions must remain enabled. No external dependencies. */
#include "checker.h"

#include <assert.h>
#include <inttypes.h>
#include <string.h>

static uint64_t bit(size_t s) { return UINT64_C(1) << s; }

static int adjacent(const mc_graph *g, size_t s, size_t t)
{
    return g->edge[s] ? (g->edge[s] & bit(t)) != 0 : s == t;
}

/* An until witness can be made simple by erasing repeated-state segments. */
static int until_path(const mc_graph *g, size_t s, uint64_t p,
                      uint64_t q, uint64_t visited)
{
    if (q & bit(s)) return 1;
    if (!(p & bit(s)) || (visited & bit(s))) return 0;
    for (size_t t = 0; t < g->n; ++t)
        if (adjacent(g, s, t) && until_path(g, t, p, q, visited | bit(s)))
            return 1;
    return 0;
}

/* n edges wholly in p repeat a state, hence give an infinite p path. */
static int long_path(const mc_graph *g, size_t s, uint64_t p, size_t left)
{
    if (!(p & bit(s))) return 0;
    if (left == 0) return 1;
    for (size_t t = 0; t < g->n; ++t)
        if (adjacent(g, s, t) && long_path(g, t, p, left - 1)) return 1;
    return 0;
}

static void distances(const mc_graph *g, unsigned d[64][64])
{
    for (size_t s = 0; s < g->n; ++s)
        for (size_t t = 0; t < g->n; ++t)
            d[s][t] = s == t ? 0 : adjacent(g, s, t) ? 1 : 1000;
    for (size_t k = 0; k < g->n; ++k)
        for (size_t s = 0; s < g->n; ++s)
            for (size_t t = 0; t < g->n; ++t)
                if (d[s][k] + d[k][t] < d[s][t])
                    d[s][t] = d[s][k] + d[k][t];
}

static void replay(const mc_graph *g, const mc_trace *trace, unsigned distance)
{
    assert(trace->length == (size_t)distance + 1);
    assert(trace->length <= g->n);
    assert(g->initial & bit(trace->state[0]));
    for (size_t i = 1; i < trace->length; ++i)
        assert(adjacent(g, trace->state[i - 1], trace->state[i]));
    assert(g->props[trace->state[trace->length - 1]] & 1);
}

static void exhaustive(void)
{
    size_t graphs = 0, safety = 0, ctl = 0;
    for (size_t n = 1; n <= 3; ++n) {
        uint64_t all = mc_universe(n);
        for (uint64_t edges = 0; edges < bit(n * n); ++edges) {
            mc_graph g = {0};
            g.n = n;
            g.initial = 1;
            for (size_t s = 0; s < n; ++s)
                for (size_t t = 0; t < n; ++t)
                    if (edges & bit(s * n + t)) g.edge[s] |= bit(t);
            ++graphs;
            for (uint64_t p = 0; p <= all; ++p) {
                uint64_t ex = 0, eg = 0, got;
                for (size_t s = 0; s < n; ++s) {
                    for (size_t t = 0; t < n; ++t)
                        if (adjacent(&g, s, t) && (p & bit(t))) ex |= bit(s);
                    if (long_path(&g, s, p, n)) eg |= bit(s);
                }
                assert(mc_ctl(&g, MC_EX, p, 0, 1, &got) == MC_OK);
                assert(got == ex);
                assert(mc_ctl(&g, MC_EG, p, 0, n + 1, &got) == MC_OK);
                assert(got == eg);
                ctl += 2;
                for (uint64_t q = 0; q <= all; ++q) {
                    uint64_t eu = 0;
                    for (size_t s = 0; s < n; ++s)
                        if (until_path(&g, s, p, q, 0)) eu |= bit(s);
                    assert(mc_ctl(&g, MC_EU, p, q, n + 1, &got) == MC_OK);
                    assert(got == eu);
                    ++ctl;
                }
            }
            unsigned d[64][64];
            distances(&g, d);
            for (uint64_t initial = 1; initial <= all; ++initial) {
                g.initial = initial;
                for (uint64_t bad = 0; bad <= all; ++bad) {
                    unsigned best = 1000;
                    for (size_t s = 0; s < n; ++s) {
                        g.props[s] = (bad & bit(s)) ? 1 : 0;
                        for (size_t t = 0; t < n; ++t)
                            if ((initial & bit(s)) && (bad & bit(t)) &&
                                d[s][t] < best) best = d[s][t];
                    }
                    assert(mc_atoms(&g, 1) == bad);
                    mc_trace trace;
                    mc_status status = mc_safety(&g, 1, n, &trace);
                    assert(status == (best < 1000 ? MC_UNSAFE : MC_SAFE));
                    if (status == MC_UNSAFE) replay(&g, &trace, best);
                    else assert(trace.length == 0);
                    ++safety;
                }
            }
        }
    }
    printf("exhaustive: %zu graphs, %zu safety queries, %zu CTL queries\n",
           graphs, safety, ctl);
}

static mc_status parse(const char *text, mc_graph *g)
{
    FILE *in = tmpfile();
    assert(in);
    assert(fputs(text, in) >= 0);
    rewind(in);
    mc_status status = mc_read(in, g);
    assert(fclose(in) == 0);
    return status;
}

static void invalid_and_parser(void)
{
    const char *bad[] = {
        "", "1", "1 1", "0 0 0", "1 0 0", "1 2 0", "2 2 0 0 0 0 0",
        "1 1 0 1 0", "1 1 0 0", "1 1 1 0 0 0", "1 1 1 0 0 0 1",
        "1 1 1 0 0 1 0", "1 1 0 0 0 junk", "1 1 0 0 -1",
        "1 1 0 0 +1", "1 1 0 0 0x1", "1 1 0 0 1x",
        "1 1 0 0 18446744073709551616", "18446744073709551616 1 0",
        "1 1 0 0 0 0"
    };
    mc_graph g;
    for (size_t i = 0; i < sizeof bad / sizeof bad[0]; ++i) {
        memset(&g, 0xff, sizeof g);
        assert(parse(bad[i], &g) == MC_INVALID);
        assert(g.n == 0 && g.initial == 0 && g.edge[0] == 0);
    }
    assert(parse("65 1 0", &g) == MC_LIMIT);
    assert(parse("1 1 4097", &g) == MC_LIMIT);
    assert(parse("18446744073709551615 1 0", &g) == MC_LIMIT);
    assert(parse(" 1\t1\n2 0 18446744073709551615 0 0 0 0 \n", &g) == MC_OK);
    assert(g.props[0] == UINT64_MAX && g.edge[0] == 1);
    assert(mc_atoms(&g, UINT64_C(1) << 63) == 1);
    assert(parse("1 1 0 0 0", &g) == MC_OK);
    assert(g.edge[0] == 0);
    assert(mc_universe(0) == 0 && mc_universe(65) == 0);
    assert(mc_universe(64) == UINT64_MAX);
    assert(mc_validate(NULL) == MC_INVALID);
    assert(mc_read(NULL, &g) == MC_INVALID && g.n == 0);
    assert(mc_read(NULL, NULL) == MC_INVALID);
    g.n = 65;
    assert(mc_validate(&g) == MC_LIMIT);
    g.n = 1;
    assert(mc_validate(&g) == MC_INVALID);
    g.initial = 2;
    assert(mc_validate(&g) == MC_INVALID);
    g.initial = 1;
    g.edge[0] = 2;
    assert(mc_validate(&g) == MC_INVALID);
    assert(mc_atoms(&g, 1) == 0);
    mc_trace trace = {0};
    trace.length = 99;
    assert(mc_safety(&g, 1, 1, &trace) == MC_INVALID && trace.length == 0);
    uint64_t out = 99;
    assert(mc_ctl(&g, MC_EX, 1, 0, 1, &out) == MC_INVALID && out == 0);
    g.edge[0] = 0;
    assert(mc_safety(&g, 1, 1, NULL) == MC_INVALID);
    assert(mc_ctl(&g, MC_EX, 1, 0, 1, NULL) == MC_INVALID);
    assert(mc_ctl(&g, (mc_operator)99, 1, 0, 1, &out) == MC_INVALID);
    assert(mc_ctl(&g, MC_EX, 2, 0, 1, &out) == MC_INVALID);
    assert(mc_ctl(&g, MC_EX, 1, 2, 1, &out) == MC_INVALID);
    puts("parser/API: 20 malformed streams, capacity and null/bit bounds passed");
}

static void boundaries(void)
{
    mc_graph g = {0};
    g.n = 64;
    g.initial = 1;
    for (size_t s = 0; s < 63; ++s) g.edge[s] = bit(s + 1);
    g.props[63] = 1;
    mc_trace trace;
    uint64_t out;
    assert(mc_safety(&g, 1, 0, &trace) == MC_LIMIT && trace.length == 0);
    assert(mc_safety(&g, 1, 63, &trace) == MC_LIMIT && trace.length == 0);
    assert(mc_safety(&g, 1, 64, &trace) == MC_UNSAFE);
    replay(&g, &trace, 63);
    assert(mc_safety(&g, 1, SIZE_MAX, &trace) == MC_UNSAFE);
    assert(mc_ctl(&g, MC_EX, bit(63), 0, 0, &out) == MC_LIMIT && out == 0);
    assert(mc_ctl(&g, MC_EX, bit(63), 0, 1, &out) == MC_OK);
    assert(out == (bit(62) | bit(63)));
    assert(mc_ctl(&g, MC_EU, UINT64_MAX, bit(63), 63, &out) == MC_LIMIT);
    assert(out == 0);
    assert(mc_ctl(&g, MC_EU, UINT64_MAX, bit(63), 64, &out) == MC_OK);
    assert(out == UINT64_MAX);
    assert(mc_ctl(&g, MC_EG, ~bit(63), 0, 63, &out) == MC_LIMIT && out == 0);
    assert(mc_ctl(&g, MC_EG, ~bit(63), 0, 64, &out) == MC_OK && out == 0);
    assert(mc_ctl(&g, MC_EG, UINT64_MAX, 0, SIZE_MAX, &out) == MC_OK);
    assert(out == UINT64_MAX);
    g.props[63] = 0;
    assert(mc_safety(&g, 1, 63, &trace) == MC_LIMIT);
    assert(mc_safety(&g, 1, 64, &trace) == MC_SAFE);
    g.initial = bit(63);
    g.props[63] = 1;
    assert(mc_safety(&g, 1, 1, &trace) == MC_UNSAFE);
    replay(&g, &trace, 0);
    g.initial = UINT64_MAX;
    assert(mc_safety(&g, 1, 64, &trace) == MC_UNSAFE);
    replay(&g, &trace, 0);
    puts("boundaries: 64 states, bit 63, exact budgets, SIZE_MAX passed");
}

static void maximum_stream(void)
{
    FILE *in = tmpfile();
    assert(in);
    assert(fputs("64 64 4096\n", in) >= 0);
    for (size_t s = 0; s < 64; ++s) assert(fprintf(in, "%zu ", s) > 0);
    for (size_t s = 0; s < 64; ++s)
        assert(fprintf(in, "%" PRIu64 " ", bit(s)) > 0);
    for (size_t s = 0; s < 64; ++s)
        for (size_t t = 0; t < 64; ++t)
            assert(fprintf(in, "%zu %zu\n", s, t) > 0);
    rewind(in);
    mc_graph g;
    assert(mc_read(in, &g) == MC_OK);
    assert(fclose(in) == 0);
    assert(g.n == 64 && g.initial == UINT64_MAX);
    for (size_t s = 0; s < 64; ++s) {
        assert(g.props[s] == bit(s));
        assert(g.edge[s] == UINT64_MAX);
    }
    uint64_t out;
    assert(mc_ctl(&g, MC_EX, bit(63), 0, 1, &out) == MC_OK);
    assert(out == UINT64_MAX);
    assert(mc_ctl(&g, MC_EG, bit(63), 0, 2, &out) == MC_OK);
    assert(out == bit(63));
    puts("maximum stream: 64 initial states, 4096 edges, all proposition bits passed");
}

static void case_study(const char *directory)
{
    const char *names[] = {"stale.graph", "fixed.graph"};
    const uint64_t expected[][3] = {{12, 63, 55}, {8, 8, 55}};
    for (size_t i = 0; i < 2; ++i) {
        char path[1024];
        int len = snprintf(path, sizeof path, "%s/%s", directory, names[i]);
        assert(len > 0 && (size_t)len < sizeof path);
        FILE *in = fopen(path, "r");
        assert(in);
        mc_graph g;
        assert(mc_read(in, &g) == MC_OK);
        assert(fclose(in) == 0);
        mc_trace trace;
        assert(mc_safety(&g, 1, 64, &trace) == (i == 0 ? MC_UNSAFE : MC_SAFE));
        if (i == 0) {
            replay(&g, &trace, 3);
            for (size_t s = 0; s < 4; ++s) assert(trace.state[s] == s);
        } else assert(trace.length == 0);
        uint64_t out, all = mc_universe(g.n), bad = mc_atoms(&g, 1);
        assert(mc_ctl(&g, MC_EX, bad, 0, 7, &out) == MC_OK);
        assert(out == expected[i][0]);
        assert(mc_ctl(&g, MC_EU, all, bad, 7, &out) == MC_OK);
        assert(out == expected[i][1]);
        assert(mc_ctl(&g, MC_EG, all & ~bad, 0, 7, &out) == MC_OK);
        assert(out == expected[i][2]);
    }
    puts("case study: shortest stale-code trace and repaired model passed");
}

int main(int argc, char **argv)
{
    assert(argc == 2);
    exhaustive();
    invalid_and_parser();
    boundaries();
    maximum_stream();
    case_study(argv[1]);
    puts("all checks passed");
    return 0;
}
