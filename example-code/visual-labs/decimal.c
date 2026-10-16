/* Chapter 3: checked decimal scanning. C11. */
#include "visual_labs.h"
#include <limits.h>

bool read_decimal(const char *text, size_t length,
                  size_t *cursor, unsigned *value)
{
    size_t start = *cursor, pos = start;
    unsigned number = 0;
    if (start > length)
        return false;
    while (pos < length && text[pos] >= '0' && text[pos] <= '9') {
        unsigned digit = (unsigned)(text[pos] - '0');
        if (number > (UINT_MAX - digit) / 10U)
            return false;
        number = number * 10U + digit;
        ++pos;
    }
    if (pos == start)
        return false;
    *cursor = pos;
    *value = number;
    return true;
}
