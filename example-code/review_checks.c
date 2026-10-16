/* Checked examples for Chapters 12, 18, 28, and 31. C11, no dependencies.
   Build with: cc -std=c11 -Wall -Wextra -Wpedantic -Werror
   example-code/review_checks.c -o /tmp/opencode/review-checks
   Keep assertions enabled. Block i in a bitset uses bit (i-1). */
#include <assert.h>
#include <stdbool.h>
#include <stdio.h>

static unsigned bit(unsigned i) { return 1U << (i - 1U); }

static void frontiers(void)
{
    const unsigned pred[7] = {
        0, 0, (1U << 0) | (1U << 4), 1U << 1,
        1U << 1, (1U << 2) | (1U << 3), 1U << 4
    };
    unsigned dom[7] = {0, 1, 63, 63, 63, 63, 63};
    bool changed;
    do {
        changed = false;
        for (unsigned b = 2; b <= 6; ++b) {
            unsigned incoming = 63;
            for (unsigned p = 1; p <= 6; ++p)
                if ((pred[b] & bit(p)) != 0)
                    incoming &= dom[p];
            unsigned next = incoming | bit(b);
            changed |= next != dom[b];
            dom[b] = next;
        }
    } while (changed);
    const unsigned expected_dom[7] = {0, 1, 3, 7, 11, 19, 51};
    const unsigned expected_df[7] = {0, 0, 2, 16, 16, 2, 0};
    for (unsigned n = 1; n <= 6; ++n) {
        unsigned df = 0;
        assert(dom[n] == expected_dom[n]);
        for (unsigned y = 1; y <= 6; ++y) {
            bool dominates_pred = false;
            for (unsigned p = 1; p <= 6; ++p)
                if ((pred[y] & bit(p)) && (dom[p] & bit(n)))
                    dominates_pred = true;
            bool strictly_dominates = n != y && (dom[y] & bit(n));
            if (dominates_pred && !strictly_dominates)
                df |= bit(y);
        }
        assert(df == expected_df[n]);
    }
}

static void self_loop(void)
{
    /* Entry 1 -> header 2, with 2 -> 2: entry must not join the loop. */
    const unsigned pred[3] = {0, 0, 3};
    unsigned header = 2, latch = 2;
    unsigned body = bit(header) | bit(latch);
    unsigned work = bit(latch) & ~bit(header);
    while (work != 0) {
        unsigned node = 1;
        while (!(work & bit(node)))
            ++node;
        work &= ~bit(node);
        unsigned fresh = pred[node] & ~body;
        body |= fresh;
        work |= fresh;
    }
    assert(body == bit(2));
}

static void parallel_copy(void)
{
    for (unsigned a = 0; a < 256; ++a)
        for (unsigned b = 0; b < 256; ++b) {
            unsigned r1 = a, r2 = b;
            unsigned temp = r1;
            r1 = r2;
            r2 = temp;
            assert(r1 == b && r2 == a);
        }
}

static bool stack_step(unsigned depth, unsigned pops, unsigned pushes,
                       unsigned *after)
{
    if (depth < pops)
        return false;
    *after = depth - pops + pushes;
    return true;
}

int main(void)
{
    frontiers();
    self_loop();
    parallel_copy();
    unsigned after = 99;
    assert(!stack_step(1, 2, 1, &after) && after == 99);
    assert(stack_step(2, 2, 1, &after) && after == 1);
    /* Chapter 19: include failed header tests in both loops. */
    assert((100U + 1U) + 100U == 201U);
    assert((25U + 1U) + 25U == 51U);
    assert((25U + 1U) + 25U + (2U + 1U) + 2U == 56U);
    /* Chapter 26: producer latencies bound each consumer's issue time. */
    const unsigned issue[5] = {0, 3, 1, 5, 7};
    assert(issue[1] >= issue[0] + 3U);
    assert(issue[3] >= issue[1] + 2U);
    assert(issue[3] >= issue[2] + 1U);
    assert(issue[4] >= issue[3] + 2U);
    assert(issue[4] + 1U == 8U);
    /* Chapter 27: two-register schedule with b spilled around c,d. */
    for (unsigned a = 0; a < 32; ++a)
        for (unsigned b = 0; b < 32; ++b) {
            unsigned r1 = a, r2 = b, slot = r2;
            r2 = r1 + r2;
            r2 = r2 * r1;
            r1 = slot;
            r2 = r2 + r1;
            assert(r2 == (a + b) * a + b);
        }
    puts("review checks: graph, stack, copy, count, schedule, allocation passed");
    return 0;
}
