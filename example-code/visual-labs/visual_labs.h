/* Companion code for Chapters 3, 28, 31, and 32. Requires C11.
   Pointers must refer to live objects of the documented sizes.
   Output objects must not alias input arrays or one another. */
#ifndef VISUAL_LABS_H
#define VISUAL_LABS_H
#include <stdbool.h>
#include <stddef.h>

/* Read decimal digits at *cursor; reject missing digits or overflow.
   On failure both output objects retain their original values. */
bool read_decimal(const char *text, size_t length,
                  size_t *cursor, unsigned *value);

enum { REGISTER_COUNT = 8 };
typedef struct { size_t destination, source; } Copy;
/* Destinations must be unique; all indices must be below REGISTER_COUNT.
   Rejected copies leave regs unchanged. */
bool parallel_copy(unsigned regs[REGISTER_COUNT],
                   const Copy *copies, size_t count);

/* Compute a depth transition; failure leaves *after unchanged. */
bool depth_step(size_t before, size_t pops, size_t pushes,
                size_t capacity, size_t *after);

/* Round to a nonzero byte alignment; failure leaves *result unchanged. */
bool align_offset(size_t offset, size_t alignment, size_t *result);
#endif
