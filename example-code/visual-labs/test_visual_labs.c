/* Tests of the book's visual-laboratory C listings. C11.
   Keep assertions enabled; see README.md for build commands. */
#include "visual_labs.h"
#include <assert.h>
#include <limits.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

static void test_decimal(void)
{
    const char text[] = "x<=10";
    size_t cursor = 3;
    unsigned value = 99;
    assert(read_decimal(text, 5, &cursor, &value));
    assert(cursor == 5 && value == 10);
    cursor = 0;
    assert(!read_decimal(text, 5, &cursor, &value));
    assert(cursor == 0 && value == 10);
    assert(read_decimal("007;", 4, &cursor, &value));
    assert(cursor == 3 && value == 7);
    /* Enough room for a decimal representation, one extra digit, and NUL. */
    char limit[sizeof(unsigned) * CHAR_BIT + 3];
    int written = snprintf(limit, sizeof limit, "%u", UINT_MAX);
    assert(written > 0 && (size_t)written + 2 <= sizeof limit);
    cursor = 0;
    assert(read_decimal(limit, (size_t)written, &cursor, &value));
    assert(value == UINT_MAX);
    limit[written] = '0';
    cursor = 0;
    value = 42;
    assert(!read_decimal(limit, (size_t)written + 1, &cursor, &value));
    assert(cursor == 0 && value == 42);
    cursor = 6;
    assert(!read_decimal(text, 5, &cursor, &value));
    assert(cursor == 6 && value == 42);
}

static void test_copies(void)
{
    const Copy swap[] = {{0, 1}, {1, 0}};
    for (unsigned a = 0; a < 256; ++a)
        for (unsigned b = 0; b < 256; ++b) {
            unsigned regs[REGISTER_COUNT] = {a, b};
            assert(parallel_copy(regs, swap, 2));
            assert(regs[0] == b && regs[1] == a);
        }
    unsigned regs[REGISTER_COUNT] = {10, 20, 30, 40, 50, 60, 70, 80};
    const Copy cycle[] = {{0, 1}, {1, 2}, {2, 0}, {3, 0}};
    assert(parallel_copy(regs, cycle, 4));
    assert(regs[0] == 20 && regs[1] == 30);
    assert(regs[2] == 10 && regs[3] == 10 && regs[7] == 80);
    unsigned before[REGISTER_COUNT];
    memcpy(before, regs, sizeof regs);
    const Copy duplicate[] = {{0, 1}, {0, 2}};
    const Copy bad_source[] = {{0, 1}, {1, REGISTER_COUNT}};
    assert(!parallel_copy(regs, duplicate, 2));
    assert(memcmp(before, regs, sizeof regs) == 0);
    assert(!parallel_copy(regs, bad_source, 2));
    assert(memcmp(before, regs, sizeof regs) == 0);
    assert(!parallel_copy(regs, swap, REGISTER_COUNT + 1));
    assert(parallel_copy(regs, swap, 0));
    assert(memcmp(before, regs, sizeof regs) == 0);
}

static void test_depths(void)
{
    for (size_t cap = 0; cap <= 8; ++cap)
        for (size_t before = 0; before <= 9; ++before)
            for (size_t pops = 0; pops <= 9; ++pops)
                for (size_t pushes = 0; pushes <= 9; ++pushes) {
                    int expected = (int)before - (int)pops + (int)pushes;
                    bool valid = before <= cap && before >= pops &&
                                 expected <= (int)cap;
                    size_t after = 99;
                    assert(depth_step(before, pops, pushes, cap, &after) == valid);
                    assert(after == (valid ? (size_t)expected : 99));
                }
    size_t after = 42;
    assert(!depth_step(SIZE_MAX, 0, 1, SIZE_MAX, &after));
    assert(after == 42);
    assert(depth_step(SIZE_MAX, 1, 1, SIZE_MAX, &after));
    assert(after == SIZE_MAX);
}

static void test_layout(void)
{
    for (size_t align = 1; align <= 32; ++align)
        for (size_t offset = 0; offset <= 255; ++offset) {
            size_t expected = offset, result = 999;
            while (expected % align != 0)
                ++expected;
            assert(align_offset(offset, align, &result));
            assert(result == expected);
        }
    size_t result = 99;
    assert(align_offset(20, 8, &result) && result == 24);
    assert(align_offset(33, 8, &result) && result == 40);
    assert(!align_offset(20, 0, &result) && result == 40);
    assert(!align_offset(SIZE_MAX, 2, &result) && result == 40);
    assert(align_offset(SIZE_MAX, 1, &result) && result == SIZE_MAX);
}

int main(void)
{
    test_decimal();
    test_copies();
    test_depths();
    test_layout();
    puts("visual labs: decimal, copy, stack depth, and layout checks passed");
    return 0;
}
