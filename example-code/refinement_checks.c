/* Bounded differential checks for the third manuscript pass (Chapters 16,
   20, and 36). C11, no dependencies. Keep assertions enabled.
   cc -std=c11 -Wall -Wextra -Wpedantic -Werror this-file.c -o checks
   These small reference models do not implement a production optimizer. */
#include <assert.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>

enum { REGS = 4, LIMIT = 32, CELLS = 8, CONSTRAINTS = 5 };
enum Op { LITERAL, COPY, ADD, MUL };
typedef struct { enum Op op; unsigned dst, a, b; } Inst;
typedef struct { enum Op op; unsigned a, b; } Key;

static bool equal_key(Key a, Key b)
{
    return a.op == b.op && a.a == b.a && a.b == b.b;
}

/* Linear tables make the bounded model readable; no hash-table cost claim. */
static void number_block(const Inst *in, Inst *out, unsigned count)
{
    Key keys[LIMIT];
    unsigned key_number[LIMIT], key_count = 0, next = REGS;
    unsigned var[REGS] = {0, 1, 2, 3};
    unsigned holders[LIMIT] = {1, 2, 4, 8};
    bool literal[LIMIT] = {false};
    unsigned value[LIMIT] = {0};
    for (unsigned i = 0; i < count; ++i) {
        Inst ins = in[i];
        unsigned n;
        out[i] = ins;
        if (ins.op == COPY) {
            n = var[ins.a];
        } else {
            Key key = {ins.op, ins.a, 0};
            if (ins.op != LITERAL) {
                key.a = var[ins.a];
                key.b = var[ins.b];
                if (literal[key.a] && literal[key.b]) {
                    unsigned a = value[key.a], b = value[key.b];
                    key = (Key){LITERAL,
                        (ins.op == ADD ? a + b : a * b) & 255U, 0};
                } else if (key.a > key.b) {
                    unsigned tmp = key.a;
                    key.a = key.b;
                    key.b = tmp;
                }
            }
            unsigned k = 0;
            while (k < key_count && !equal_key(keys[k], key))
                ++k;
            if (k == key_count) {
                assert(next < LIMIT && key_count < LIMIT);
                keys[k] = key;
                key_number[k] = next++;
                ++key_count;
                if (key.op == LITERAL) {
                    literal[key_number[k]] = true;
                    value[key_number[k]] = key.a;
                }
            }
            n = key_number[k];
        }
        if (literal[n]) {
            out[i] = (Inst){LITERAL, ins.dst, value[n], 0};
        } else if (holders[n] != 0) {
            unsigned source = 0;
            while ((holders[n] & (1U << source)) == 0)
                ++source;
            out[i] = (Inst){COPY, ins.dst, source, 0};
        }
        holders[var[ins.dst]] &= ~(1U << ins.dst);
        var[ins.dst] = n;
        holders[n] |= 1U << ins.dst;
    }
}

static void execute(const Inst *code, unsigned count, unsigned r[REGS])
{
    for (unsigned i = 0; i < count; ++i) {
        Inst x = code[i];
        switch (x.op) {
        case LITERAL: r[x.dst] = x.a; break;
        case COPY: r[x.dst] = r[x.a]; break;
        case ADD: r[x.dst] = (r[x.a] + r[x.b]) & 255U; break;
        case MUL: r[x.dst] = (r[x.a] * r[x.b]) & 255U; break;
        }
    }
}

static void compare(const Inst *code, unsigned count, unsigned input_limit)
{
    Inst optimized[LIMIT];
    number_block(code, optimized, count);
    for (unsigned a = 0; a < input_limit; ++a)
        for (unsigned b = 0; b < input_limit; ++b) {
            unsigned expected[REGS] = {a, b, 17, 93};
            unsigned actual[REGS];
            memcpy(actual, expected, sizeof actual);
            execute(code, count, expected);
            execute(optimized, count, actual);
            assert(memcmp(actual, expected, sizeof actual) == 0);
        }
}

static void value_numbering(void)
{
    /* Overwritten sole representative, surviving copy, self-update, and
       an old expression key recovered through another operand holder. */
    const Inst fixtures[][5] = {
        {{ADD,2,0,1}, {LITERAL,2,0,0}, {ADD,3,0,1}},
        {{ADD,2,0,1}, {COPY,3,2,0}, {LITERAL,2,0,0}, {ADD,2,0,1}},
        {{ADD,0,0,1}, {ADD,2,0,1}, {ADD,3,1,0}},
        {{COPY,3,0,0}, {ADD,2,0,1}, {LITERAL,0,0,0}, {ADD,0,3,1}},
        {{LITERAL,2,2,0}, {LITERAL,3,3,0}, {ADD,2,2,3}, {MUL,3,2,3}}
    };
    const unsigned lengths[] = {3, 4, 3, 4, 4};
    for (unsigned i = 0; i < sizeof lengths / sizeof lengths[0]; ++i)
        compare(fixtures[i], lengths[i], 256);
    Inst rewritten[LIMIT];
    number_block(fixtures[0], rewritten, 3);
    assert(rewritten[2].op == ADD); /* no live holder: must recompute */
    number_block(fixtures[1], rewritten, 4);
    assert(rewritten[3].op == COPY && rewritten[3].a == 3);

    const Inst choices[] = {
        {ADD,2,0,1}, {ADD,3,1,0}, {MUL,2,0,1}, {MUL,3,1,0},
        {COPY,3,2,0}, {COPY,0,2,0}, {COPY,2,3,0}, {LITERAL,2,0,0},
        {LITERAL,0,1,0}, {ADD,0,0,1}, {ADD,2,2,3}, {MUL,3,2,3}
    };
    unsigned size = (unsigned)(sizeof choices / sizeof choices[0]);
    for (unsigned a = 0; a < size; ++a)
        for (unsigned b = 0; b < size; ++b)
            for (unsigned c = 0; c < size; ++c) {
                const Inst code[] = {choices[a], choices[b], choices[c]};
                compare(code, 3, 8);
            }
}

enum ConstraintKind { ADDRESS, INCLUDE, LOAD, STORE };
typedef struct { enum ConstraintKind kind; unsigned dst, src; } Constraint;
/* Cells 0..4 are a,b,c,d,m; object n is cell 5. */
static const Constraint constraints[CONSTRAINTS] = {
    {ADDRESS,0,4}, {INCLUDE,1,0}, {ADDRESS,2,5},
    {STORE,1,2}, {LOAD,3,0}
};

/* Independent full-sweep equations, without dependency registrations. */
static void reference_points(unsigned pts[CELLS])
{
    bool changed;
    do {
        unsigned old[CELLS];
        memcpy(old, pts, sizeof old);
        pts[0] |= 1U << 4;
        pts[1] |= pts[0];
        pts[2] |= 1U << 5;
        for (unsigned o = 0; o < CELLS; ++o) {
            if (pts[1] & (1U << o)) pts[o] |= pts[2];
            if (pts[0] & (1U << o)) pts[3] |= pts[o];
        }
        changed = memcmp(old, pts, sizeof old) != 0;
    } while (changed);
}

static void incremental_points(const unsigned order[CONSTRAINTS])
{
    unsigned pts[CELLS] = {0}, readers[CELLS] = {0};
    unsigned pending = (1U << CONSTRAINTS) - 1U, steps = 0;
    while (pending) {
        assert(++steps < 100);
        unsigned rank = 0;
        while (!(pending & (1U << order[rank]))) ++rank;
        unsigned id = order[rank], mask = 1U << id;
        pending &= ~mask;
        Constraint c = constraints[id];
        unsigned old[CELLS];
        memcpy(old, pts, sizeof old);
        if (c.kind == ADDRESS) {
            pts[c.dst] |= 1U << c.src;
        } else {
            readers[c.src] |= mask;
            if (c.kind == INCLUDE) pts[c.dst] |= pts[c.src];
            if (c.kind == LOAD)
                for (unsigned o = 0; o < CELLS; ++o)
                    if (pts[c.src] & (1U << o)) {
                        readers[o] |= mask;
                        pts[c.dst] |= pts[o];
                    }
            if (c.kind == STORE) {
                readers[c.dst] |= mask;
                for (unsigned o = 0; o < CELLS; ++o)
                    if (pts[c.dst] & (1U << o)) pts[o] |= pts[c.src];
            }
        }
        for (unsigned o = 0; o < CELLS; ++o)
            if (old[o] != pts[o]) pending |= readers[o];
    }
    unsigned expected[CELLS] = {0};
    reference_points(expected);
    assert(memcmp(pts, expected, sizeof pts) == 0);
    assert(pts[3] == (1U << 5)); /* d targets n through m's stored pointer */
}

static void permutations(unsigned order[CONSTRAINTS], unsigned pos)
{
    if (pos == CONSTRAINTS) {
        incremental_points(order);
        return;
    }
    for (unsigned i = pos; i < CONSTRAINTS; ++i) {
        unsigned t = order[pos]; order[pos] = order[i]; order[i] = t;
        permutations(order, pos + 1);
        t = order[pos]; order[pos] = order[i]; order[i] = t;
    }
}

static void tiled_product(void)
{
    unsigned plain[8][8] = {{0}}, tiled[8][8] = {{0}}, updates = 0;
    for (unsigned i = 0; i < 8; ++i)
        for (unsigned j = 0; j < 8; ++j)
            for (unsigned k = 0; k < 8; ++k)
                plain[i][j] += (i + 2 * k) * (3 * k + j);
    for (unsigned ii = 0; ii < 8; ii += 4)
        for (unsigned jj = 0; jj < 8; jj += 4)
            for (unsigned kk = 0; kk < 8; kk += 4)
                for (unsigned i = ii; i < ii + 4; ++i)
                    for (unsigned j = jj; j < jj + 4; ++j)
                        for (unsigned k = kk; k < kk + 4; ++k) {
                            tiled[i][j] += (i + 2 * k) * (3 * k + j);
                            ++updates;
                        }
    assert(updates == 512 && memcmp(plain, tiled, sizeof plain) == 0);
}

int main(void)
{
    unsigned order[CONSTRAINTS] = {0, 1, 2, 3, 4};
    value_numbering();
    permutations(order, 0);
    tiled_product();
    puts("refinement checks: value holders, dynamic points-to, tiling passed");
    return 0;
}
