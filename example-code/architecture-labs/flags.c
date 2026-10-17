/* Source chapter 64. Independent x86-64 hardware oracle for ADD AL semantics.
   GCC/Clang x86 inline assembly extension; writes all 65,536 byte pairs.
   No generated machine code or executable-memory allocation is needed. */
#include <stdio.h>
#include <stdint.h>
int main(void)
{
    for (unsigned a=0; a<256; ++a) for (unsigned b=0; b<256; ++b) {
        uint8_t result=(uint8_t)a, rhs=(uint8_t)b;
        uint64_t flags;
        __asm__ volatile ("addb %2, %0\n\tpushfq\n\tpopq %1"
                          : "+&a"(result), "=r"(flags) : "q"(rhs) : "cc", "memory");
        /* Standalone function, no red-zone locals live across pushfq/popq.
           Build with -mno-red-zone to make that condition explicit. */
        printf("%u %u %u %llu\n",a,b,(unsigned)result,(unsigned long long)flags);
    }
    return ferror(stdout) ? 2 : 0;
}
