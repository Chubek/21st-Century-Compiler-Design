/* Chapter 32: round offsets without overflowing host size_t. C11. */
#include "visual_labs.h"
#include <stdint.h>

bool align_offset(size_t offset, size_t alignment, size_t *result)
{
    if (alignment == 0)
        return false;
    size_t remainder = offset % alignment;
    size_t padding = remainder == 0 ? 0 : alignment - remainder;
    if (offset > SIZE_MAX - padding)
        return false;
    *result = offset + padding;
    return true;
}
