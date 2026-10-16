/* Chapter 31: check operand arity before the net stack effect. C11. */
#include "visual_labs.h"

bool depth_step(size_t before, size_t pops, size_t pushes,
                size_t capacity, size_t *after)
{
    if (before > capacity || before < pops)
        return false;
    size_t remaining = before - pops;
    if (pushes > capacity - remaining)
        return false;
    *after = remaining + pushes;
    return true;
}
