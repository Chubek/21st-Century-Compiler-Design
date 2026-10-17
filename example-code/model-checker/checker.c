/* Chapter 48: C11 checker implementation; see README.md for contracts. */
#include "checker.h"

#include <ctype.h>
#include <string.h>

static uint64_t bit(size_t s)
{
    return UINT64_C(1) << s;
}

uint64_t mc_universe(size_t n)
{
    if (n == 0 || n > MC_MAX_STATES) return 0;
    return n == MC_MAX_STATES ? UINT64_MAX : bit(n) - 1;
}

mc_status mc_validate(const mc_graph *g)
{
    if (!g || g->n == 0) return MC_INVALID;
    if (g->n > MC_MAX_STATES) return MC_LIMIT;
    uint64_t all = mc_universe(g->n);
    if (!g->initial || (g->initial & ~all)) return MC_INVALID;
    for (size_t s = 0; s < g->n; ++s)
        if (g->edge[s] & ~all) return MC_INVALID;
    return MC_OK;
}

/* Decimal tokens only: no signs, prefixes, suffixes, or silent overflow. */
static int number(FILE *in, uint64_t *out)
{
    int c;
    do { c = fgetc(in); } while (c != EOF && isspace((unsigned char)c));
    if (c < '0' || c > '9') return 0;
    uint64_t v = 0;
    do {
        unsigned d = (unsigned)(c - '0');
        if (v > (UINT64_MAX - d) / 10) return 0;
        v = v * 10 + d;
        c = fgetc(in);
    } while (c >= '0' && c <= '9');
    if (c != EOF && !isspace((unsigned char)c)) return 0;
    if (ferror(in)) return 0;
    *out = v;
    return 1;
}

mc_status mc_read(FILE *in, mc_graph *out)
{
    if (!out) return MC_INVALID;
    memset(out, 0, sizeof *out);
    if (!in) return MC_INVALID;
    mc_graph g = {0};
    uint64_t n, k, m, x, y;
    if (!number(in, &n) || !number(in, &k) || !number(in, &m))
        return MC_INVALID;
    if (n == 0 || k == 0 || k > n) return MC_INVALID;
    if (n > MC_MAX_STATES || m > MC_MAX_STATES * MC_MAX_STATES)
        return MC_LIMIT;
    g.n = (size_t)n;
    for (uint64_t i = 0; i < k; ++i) {
        if (!number(in, &x) || x >= n || (g.initial & bit((size_t)x)))
            return MC_INVALID;
        g.initial |= bit((size_t)x);
    }
    for (size_t s = 0; s < g.n; ++s)
        if (!number(in, &g.props[s])) return MC_INVALID;
    for (uint64_t i = 0; i < m; ++i) {
        if (!number(in, &x) || !number(in, &y) || x >= n || y >= n)
            return MC_INVALID;
        g.edge[x] |= bit((size_t)y);
    }
    int c;
    do { c = fgetc(in); } while (c != EOF && isspace((unsigned char)c));
    if (c != EOF || ferror(in)) return MC_INVALID;
    *out = g;
    return MC_OK;
}

static uint64_t successors(const mc_graph *g, size_t s)
{
    return g->edge[s] ? g->edge[s] : bit(s);
}

uint64_t mc_atoms(const mc_graph *g, uint64_t mask)
{
    uint64_t result = 0;
    if (mc_validate(g) != MC_OK) return 0;
    for (size_t s = 0; s < g->n; ++s)
        if (g->props[s] & mask) result |= bit(s);
    return result;
}

mc_status mc_safety(const mc_graph *g, uint64_t bad_mask,
                    size_t max_visits, mc_trace *out)
{
    if (!out) return MC_INVALID;
    memset(out, 0, sizeof *out);
    mc_status status = mc_validate(g);
    if (status != MC_OK) return status;
    size_t queue[MC_MAX_STATES], parent[MC_MAX_STATES];
    size_t head = 0, tail = 0, visits = 0;
    uint64_t seen = g->initial;
    for (size_t s = 0; s < g->n; ++s) {
        if (!(seen & bit(s))) continue;
        parent[s] = s;
        queue[tail++] = s;
    }
    while (head < tail) {
        if (visits == max_visits) return MC_LIMIT;
        ++visits;
        size_t s = queue[head++];
        if (g->props[s] & bad_mask) {
            size_t v = s;
            do {
                out->state[out->length++] = v;
                if (parent[v] == v) break;
                v = parent[v];
            } while (1);
            for (size_t i = 0; i < out->length / 2; ++i) {
                size_t j = out->length - 1 - i, tmp = out->state[i];
                out->state[i] = out->state[j];
                out->state[j] = tmp;
            }
            return MC_UNSAFE;
        }
        uint64_t next = successors(g, s);
        for (size_t t = 0; t < g->n; ++t) {
            if (!(next & bit(t)) || (seen & bit(t))) continue;
            seen |= bit(t);
            parent[t] = s;
            queue[tail++] = t;
        }
    }
    return MC_SAFE;
}

static uint64_t pre(const mc_graph *g, uint64_t set)
{
    uint64_t result = 0;
    for (size_t s = 0; s < g->n; ++s)
        if (successors(g, s) & set) result |= bit(s);
    return result;
}

mc_status mc_ctl(const mc_graph *g, mc_operator op,
                 uint64_t p, uint64_t q, size_t max_rounds,
                 uint64_t *out)
{
    if (!out) return MC_INVALID;
    *out = 0;
    mc_status status = mc_validate(g);
    if (status != MC_OK) return status;
    if ((op != MC_EX && op != MC_EU && op != MC_EG) ||
        ((p | q) & ~mc_universe(g->n))) return MC_INVALID;
    uint64_t set = op == MC_EU ? q : p;
    for (size_t round = 0; round < max_rounds; ++round) {
        uint64_t next = pre(g, set);
        if (op == MC_EU) next = q | (p & next);
        if (op == MC_EG) next = p & next;
        if (op == MC_EX || next == set) {
            *out = next;
            return MC_OK;
        }
        set = next;
    }
    return MC_LIMIT;
}

const char *mc_status_name(mc_status status)
{
    switch (status) {
    case MC_OK: return "ok";
    case MC_SAFE: return "safe";
    case MC_UNSAFE: return "unsafe";
    case MC_INVALID: return "invalid";
    case MC_LIMIT: return "resource-limit";
    }
    return "invalid";
}
