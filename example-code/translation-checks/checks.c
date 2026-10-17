/* Companion to Verifying Compiler Transformations (source ch47). C11.
   Exhaustive byte-domain observations; bounded unrolling checks.
   Keep assertions enabled. No dependencies beyond the C standard library. */
#include <assert.h>
#include <stdbool.h>
#include <stdio.h>

typedef struct { bool trapped; unsigned value; } Outcome;

static Outcome source_division(unsigned condition, unsigned divisor)
{
    if (condition == 0)
        return (Outcome){false, 0};
    if (divisor == 0)
        return (Outcome){true, 0};
    return (Outcome){false, 18U / divisor};
}

static Outcome target_division(unsigned condition, unsigned divisor)
{
    if (divisor == 0)
        return (Outcome){true, 0};
    unsigned quotient = 18U / divisor;
    return (Outcome){false, condition != 0 ? quotient : 0};
}

static bool same(Outcome left, Outcome right)
{
    return left.trapped == right.trapped &&
           (left.trapped || left.value == right.value);
}

static void check_unrolling(unsigned n)
{
    unsigned i = 0, s = 0, j = 0, t = 0;
    while (j + 1U < n) {
        t = t + j + (j + 1U);
        j += 2U;
        for (unsigned step = 0; step < 2; ++step) {
            assert(i < n);
            s += i;
            ++i;
        }
        assert(i == j && s == t);
    }
    if (j < n) {
        t += j;
        ++j;
        assert(i < n);
        s += i;
        ++i;
    }
    assert(i == n && j == n && s == t);
    unsigned reference = 0;
    for (unsigned k = 0; k < n; ++k)
        reference += k;
    assert(t == reference);
}

int main(void)
{
    unsigned doubling_fail = 0, comparison_fail = 0;
    unsigned hoist_fail = 0, guarded_fail = 0, guarded_inputs = 0;
    for (unsigned x = 0; x < 256; ++x) {
        if ((2U * x) % 256U != (x + x) % 256U)
            ++doubling_fail;
        if (!(((x + 1U) % 256U) > x)) {
            ++comparison_fail;
            assert(x == 255);
        }
    }
    for (unsigned c = 0; c < 2; ++c) {
        for (unsigned d = 0; d < 256; ++d) {
            bool equal = same(source_division(c, d), target_division(c, d));
            if (!equal) {
                ++hoist_fail;
                assert(c == 0 && d == 0);
            }
            if (c == 1 || d != 0) {
                ++guarded_inputs;
                if (!equal)
                    ++guarded_fail;
            }
        }
    }
    assert(doubling_fail == 0 && comparison_fail == 1);
    assert(hoist_fail == 1 && guarded_fail == 0 && guarded_inputs == 511);
    /* At n <= 64, every sum fits even the minimum C unsigned range. */
    for (unsigned n = 0; n <= 64; ++n)
        check_unrolling(n);
    puts("doubling: 256 inputs, 0 mismatches");
    puts("successor comparison: 256 inputs, 1 mismatch (x=255)");
    puts("division hoist: 512 inputs, 1 mismatch (c=0,d=0)");
    puts("guarded division hoist: 511 inputs, 0 mismatches");
    puts("unrolling: 65 bounds, all paired states and results agree");
    return 0;
}
