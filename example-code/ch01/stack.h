/* Chapter 1: checked stack-code translation and execution. C11. */
#ifndef CH01_STACK_H
#define CH01_STACK_H
#include "expr.h"
#include <stdbool.h>
#include <stddef.h>

enum { CODE_CAP = 64 };
typedef enum { PUSH, GET_INPUT, ADD8, MUL8 } Opcode;
typedef struct { Opcode op; unsigned arg; } Instruction;
typedef struct {
    Instruction items[CODE_CAP];
    size_t used;
} Code;

/* e obeys expr.h. code initially has used <= limit <= CODE_CAP.
   On failure, used is restored; unused array slots may change. */
bool compile_expr(const Expr *e, Code *code, size_t limit);
/* Check canonical operands, stack safety, and exactly one final value. */
bool verify_code(const Code *code, size_t *peak);
/* On failure, leave *result unchanged. x must lie in 0..255. */
bool run_code(const Code *code, unsigned x, unsigned *result);
#endif
