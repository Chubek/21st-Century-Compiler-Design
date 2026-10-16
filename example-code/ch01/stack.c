/* Chapter 1: a bounded bytecode compiler and stack machine. C11. */
#include "stack.h"
#include <assert.h>

static bool append(Code *code, size_t limit, Opcode op, unsigned arg)
{
    if (code->used == limit)
        return false;
    code->items[code->used++] = (Instruction){op, arg};
    return true;
}

static bool emit_tree(const Expr *e, Code *code, size_t limit)
{
    if (e->kind == LITERAL)
        return append(code, limit, PUSH, e->literal);
    if (e->kind == INPUT)
        return append(code, limit, GET_INPUT, 0U);
    assert(e->kind == ADD || e->kind == MUL);
    if (!emit_tree(e->left, code, limit))
        return false;
    if (!emit_tree(e->right, code, limit))
        return false;
    return append(code, limit, e->kind == ADD ? ADD8 : MUL8, 0U);
}

bool compile_expr(const Expr *e, Code *code, size_t limit)
{
    assert(e != 0 && code != 0);
    assert(code->used <= limit && limit <= CODE_CAP);
    size_t start = code->used;
    if (emit_tree(e, code, limit))
        return true;
    code->used = start;
    return false;
}

bool verify_code(const Code *code, size_t *peak)
{
    size_t depth = 0, maximum = 0;
    if (code->used > CODE_CAP)
        return false;
    for (size_t pc = 0; pc < code->used; ++pc) {
        Instruction i = code->items[pc];
        if (i.op == PUSH || i.op == GET_INPUT) {
            if ((i.op == PUSH && i.arg > 255U) ||
                (i.op == GET_INPUT && i.arg != 0U))
                return false;
            ++depth;
        } else if (i.op == ADD8 || i.op == MUL8) {
            if (i.arg != 0U || depth < 2)
                return false;
            --depth;
        } else {
            return false;
        }
        if (depth > maximum)
            maximum = depth;
    }
    if (depth != 1)
        return false;
    *peak = maximum;
    return true;
}

bool run_code(const Code *code, unsigned x, unsigned *result)
{
    unsigned stack[CODE_CAP];
    size_t sp = 0, peak;
    if (x > 255U || !verify_code(code, &peak))
        return false;
    for (size_t pc = 0; pc < code->used; ++pc) {
        Instruction i = code->items[pc];
        if (i.op == PUSH || i.op == GET_INPUT) {
            stack[sp++] = i.op == PUSH ? i.arg : x;
        } else {
            unsigned b = stack[--sp];
            unsigned a = stack[--sp];
            stack[sp++] = i.op == ADD8
                ? (a + b) % 256U : (a * b) % 256U;
        }
    }
    *result = stack[0];
    return true;
}
