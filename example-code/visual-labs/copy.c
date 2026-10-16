/* Chapter 28: simultaneous copies using saved source values. C11. */
#include "visual_labs.h"

bool parallel_copy(unsigned regs[REGISTER_COUNT],
                   const Copy *copies, size_t count)
{
    unsigned saved[REGISTER_COUNT];
    bool seen[REGISTER_COUNT] = {false};
    if (count > REGISTER_COUNT)
        return false;
    for (size_t i = 0; i < count; ++i) {
        size_t dst = copies[i].destination, src = copies[i].source;
        if (dst >= REGISTER_COUNT || src >= REGISTER_COUNT)
            return false;
        if (seen[dst])
            return false;
        seen[dst] = true;
        saved[i] = regs[src];
    }
    for (size_t i = 0; i < count; ++i)
        regs[copies[i].destination] = saved[i];
    return true;
}
