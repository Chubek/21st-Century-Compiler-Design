/* Finite examples for Logic for Compiler Reasoning and Automated Reasoning
   and Its Limits (source files ch40.tex and ch41.tex).
   C11 and the standard library only; keep assertions enabled.
   Build/run instructions and coverage limits are in README.md. */
#include <assert.h>
#include <stdbool.h>
#include <stdio.h>

static bool implies(bool a, bool b)
{
    return !a || b;
}

static bool literal(int lit, unsigned valuation)
{
    unsigned variable = (unsigned)(lit < 0 ? -lit : lit);
    assert(variable >= 1 && variable <= 6);
    bool value = (valuation & (1U << (variable - 1U))) != 0;
    return lit < 0 ? !value : value;
}

/* Each clause is zero-terminated; zero is not a variable identifier. */
static bool cnf(const int clauses[][4], unsigned count, unsigned valuation)
{
    for (unsigned i = 0; i < count; ++i) {
        bool satisfied = false;
        for (unsigned j = 0; j < 4 && clauses[i][j] != 0; ++j)
            satisfied |= literal(clauses[i][j], valuation);
        if (!satisfied)
            return false;
    }
    return true;
}

static void propositional(void)
{
    const bool expected[4][3] = {
        {true, false, true}, {true, false, true},
        {false, false, false}, {false, true, true}
    };
    unsigned counterexamples = 0;
    for (unsigned p = 0; p < 2; ++p) {
        for (unsigned q = 0; q < 2; ++q) {
            unsigned row = 2 * p + q;
            assert((!p) == expected[row][0]);
            assert((p && q) == expected[row][1]);
            assert(implies(p, q) == expected[row][2]);
            for (unsigned r = 0; r < 2; ++r) {
                bool premises = (p || q) && implies(p, r);
                if (premises && !r)
                    ++counterexamples;
                assert(!(premises && !q) || r);
            }
        }
    }
    assert(counterexamples == 1); /* p=0, q=1, r=0 */
}

static void quantifiers(void)
{
    const bool relation[2][2] = {{true, false}, {false, true}};
    bool every_use_has_definition = true;
    bool common_definition = false;
    for (unsigned u = 0; u < 2; ++u) {
        bool exists = false;
        for (unsigned d = 0; d < 2; ++d)
            exists |= relation[u][d];
        every_use_has_definition &= exists;
    }
    for (unsigned d = 0; d < 2; ++d) {
        bool all = true;
        for (unsigned u = 0; u < 2; ++u)
            all &= relation[u][d];
        common_definition |= all;
    }
    assert(every_use_has_definition && !common_definition);

    const int free_y = 1;
    bool correct_substitution = true;
    bool captured_substitution = true;
    for (int z = 0; z < 2; ++z) {
        correct_substitution &= free_y <= z;
        /* Naive substitution uses the bound value on both sides. */
        int captured_left = z;
        captured_substitution &= captured_left <= z;
    }
    assert(!correct_substitution && captured_substitution);
    assert(5 < 6); /* Witness for the existential substitution exercise. */
}

static void preconditions(void)
{
    for (int initial = -32; initial <= 32; ++initial) {
        int a = initial;
        a = a + 2;
        int b = 3 * a;
        assert((b >= 12) == (initial >= 2));
        assert((b >= 15) == (initial >= 3));

        int x = initial;
        if (x < 0)
            x = -x;
        else
            x = x + 2;
        int y = 3 * x;
        bool backward6 = implies(initial < 0, -initial >= 2)
                      && implies(initial >= 0, initial + 2 >= 2);
        bool backward9 = implies(initial < 0, -initial >= 3)
                      && implies(initial >= 0, initial + 2 >= 3);
        assert((y >= 6) == backward6);
        assert((y >= 6) == (initial != -1));
        assert((y >= 9) == backward9);
        assert((y >= 9) == (initial <= -3 || initial >= 1));
        assert((initial < 0 && x == -initial)
               || (initial >= 0 && x == initial + 2));
        assert(y == 3 * x);
    }
    const bool branch = false;
    const int denominator = 0;
    assert(implies(branch, denominator != 0));
    assert(denominator == 0); /* Speculation would fault; do not divide. */
}

static void loops(void)
{
    for (int increment = 3; increment <= 5; increment += 2) {
        for (int n = 0; n <= 64; ++n) {
            int i = 0, t = 0;
            while (i < n) {
                assert(0 <= i && i <= n && t == increment * i);
                int previous_variant = n - i;
                t = t + increment;
                i = i + 1;
                assert(n - i >= 0 && n - i == previous_variant - 1);
                if (increment == 3 && n == 4)
                    assert(t == 3 * i && n - i == 4 - i);
            }
            assert(i == n && t == increment * n);
        }
    }
}

static void gates(void)
{
    for (unsigned valuation = 0; valuation < 8; ++valuation) {
        bool a = (valuation & 1U) != 0;
        bool b = (valuation & 2U) != 0;
        bool z = (valuation & 4U) != 0;
        bool conjunction = (!z || a) && (!z || b) && (z || !a || !b);
        bool disjunction = (z || !a) && (z || !b) && (!z || a || b);
        bool negation = (!z || !a) && (z || a);
        assert(conjunction == (z == (a && b)));
        assert(disjunction == (z == (a || b)));
        assert(negation == (z == !a));
    }
}

static void tseitin(void)
{
    /* Variables 1..6 are p, q, r, a, b, c. */
    const int clauses[10][4] = {
        {-4, 1, 0, 0}, {-4, 2, 0, 0}, {4, -1, -2, 0},
        {-5, -1, 0, 0}, {-5, 3, 0, 0}, {5, 1, -3, 0},
        {6, -4, 0, 0}, {6, -5, 0, 0}, {-6, 4, 5, 0},
        {6, 0, 0, 0}
    };
    const unsigned table[8][3] = {
        {0, 0, 0}, {0, 1, 1}, {0, 0, 0}, {0, 1, 1},
        {0, 0, 0}, {0, 0, 0}, {1, 0, 1}, {1, 0, 1}
    };
    unsigned total_definitions = 0, total_asserted = 0;
    for (unsigned inputs = 0; inputs < 8; ++inputs) {
        bool p = (inputs & 1U) != 0;
        bool q = (inputs & 2U) != 0;
        bool r = (inputs & 4U) != 0;
        bool original = (p && q) || (!p && r);
        unsigned extensions = 0, asserted = 0;
        unsigned row = 4U * p + 2U * q + r;
        for (unsigned auxiliary = 0; auxiliary < 8; ++auxiliary) {
            unsigned valuation = inputs | (auxiliary << 3U);
            if (cnf(clauses, 9, valuation)) {
                ++extensions;
                assert(((auxiliary >> 0U) & 1U) == table[row][0]);
                assert(((auxiliary >> 1U) & 1U) == table[row][1]);
                assert(((auxiliary >> 2U) & 1U) == table[row][2]);
            }
            if (cnf(clauses, 10, valuation))
                ++asserted;
        }
        assert(extensions == 1);
        assert(asserted == (unsigned)original);
        total_definitions += extensions;
        total_asserted += asserted;
    }
    assert(total_definitions == 8 && total_asserted == 4);

    const int conflict[4][4] = {
        {1, 2, 0, 0}, {-1, 2, 0, 0}, {-2, 3, 0, 0}, {-3, 0, 0, 0}
    };
    for (unsigned valuation = 0; valuation < 8; ++valuation)
        assert(!cnf(conflict, 4, valuation));

    /* z (variable 2) iff (p and not p), then assert z. */
    const int contradictory_root[4][4] = {
        {-2, 1, 0, 0}, {-2, -1, 0, 0}, {2, -1, 1, 0}, {2, 0, 0, 0}
    };
    for (unsigned valuation = 0; valuation < 4; ++valuation) {
        assert(!cnf(contradictory_root, 4, valuation));
        assert(cnf(contradictory_root, 3, valuation) == !(valuation & 2U));
    }
}

static void bit_vectors(void)
{
    unsigned failures = 0;
    for (unsigned x = 0; x < 16; ++x) {
        unsigned sum = (x + 3U) & 15U;
        assert((sum > x) == (x <= 12));
        if (sum <= x)
            ++failures;
    }
    assert(failures == 3);
    const unsigned inputs[] = {0, 12, 13, 14, 15};
    const unsigned sums[] = {3, 15, 0, 1, 2};
    for (unsigned i = 0; i < 5; ++i)
        assert(((inputs[i] + 3U) & 15U) == sums[i]);

    unsigned checked = 0;
    for (unsigned width = 1; width <= 8; ++width) {
        unsigned modulus = 1U << width;
        unsigned half = modulus / 2U;
        for (unsigned x = 0; x < modulus; ++x) {
            unsigned source = ((x + x) & (modulus - 1U)) / 2U;
            assert((source == x) == (x < half));
            if (x >= half)
                assert(source == x - half);
            ++checked;
        }
    }
    assert(checked == 510);
}

int main(void)
{
    propositional();
    quantifiers();
    preconditions();
    loops();
    gates();
    tseitin();
    bit_vectors();
    puts("Logic checks passed: truth tables, binding, preconditions, loops,");
    puts("gate clauses, all 64 Tseitin valuations, and 510 width/input pairs.");
    return 0;
}
