/* Chapter 1: zero multiplication must retain effects. C11. */
#include <stdio.h>

static unsigned next_value(unsigned *calls)
{
    ++*calls;
    return 7U;
}

static unsigned keep_effect(unsigned *calls)
{
    return 0U * next_value(calls);
}

int main(void)
{
    unsigned calls = 0U;
    unsigned value = keep_effect(&calls);
    printf("value=%u calls=%u\n", value, calls);
    return value == 0U && calls == 1U ? 0 : 1;
}
