/* PebbleIR companion, C11. Fixed-capacity teaching IR; no dependencies.
   IDs are function-local. Zero is not a value. Public structs permit invalid
   construction states: verify before interpreting, lowering, or optimizing.
   Calls require valid non-null pointers and exclusive access to mutable IR. */
#ifndef PEBBLEIR_H
#define PEBBLEIR_H
#include <stdbool.h>
#include <stdio.h>

enum { PIR_BLOCKS = 8, PIR_VALUES = 128, PIR_CODE = 1024, PIR_ERROR = 128 };
typedef enum { PIR_U8, PIR_I1 } PirType;
typedef enum { PIR_DEAD, PIR_INPUT, PIR_CONST, PIR_ADD, PIR_MUL,
               PIR_LT, PIR_PHI } PirOp;
typedef enum { PIR_UNSET, PIR_RET, PIR_JUMP, PIR_BRANCH } PirTermOp;
typedef struct {
    PirOp op;
    PirType type;
    unsigned block, a, b, imm, p, q;
} PirValue;
typedef struct { PirTermOp op; unsigned value, x, y; } PirTerm;
typedef struct {
    unsigned blocks, count;
    unsigned long revision;
    PirValue values[PIR_VALUES + 1];
    PirTerm terms[PIR_BLOCKS];
} Pir;
typedef struct {
    unsigned pred[PIR_BLOCKS], dom[PIR_BLOCKS];
    unsigned long revision;
    const Pir *owner;
} PirFacts;
typedef enum { PIR_OK, PIR_INVALID, PIR_FUEL } PirStatus;
typedef struct { PirStatus status; unsigned value, steps; } PirResult;
typedef enum { VM_INPUT, VM_CONST, VM_ADD, VM_MUL, VM_LT,
               VM_MOVE, VM_JUMP, VM_BRANCH, VM_RET } PirVmOp;
typedef struct { PirVmOp op; unsigned dst, a, b; } PirInsn;
typedef struct { unsigned count; PirInsn code[PIR_CODE]; } PirVm;

void pir_init(Pir *f);
unsigned pir_block(Pir *f); /* PIR_BLOCKS on failure; block zero is entry */
unsigned pir_value(Pir *f, unsigned block, PirOp op, PirType type,
                   unsigned a, unsigned b, unsigned imm,
                   unsigned p, unsigned q); /* zero on failure */
bool pir_term(Pir *f, unsigned block, PirTerm term);
bool pir_verify(const Pir *f, PirFacts *facts, char error[PIR_ERROR]);
PirResult pir_run(const Pir *f, unsigned input, unsigned fuel);
bool pir_parse(FILE *stream, Pir *out, char error[PIR_ERROR]);
bool pir_print(FILE *stream, const Pir *f);
bool pir_compile(const char *expression, Pir *out, char error[PIR_ERROR]);
bool pir_optimize(Pir *f, char error[PIR_ERROR]);
bool pir_replace(Pir *f, unsigned from, unsigned to, char error[PIR_ERROR]);
bool pir_lower(const Pir *f, PirVm *out, char error[PIR_ERROR]);
PirResult pir_vm_run(const PirVm *vm, unsigned input, unsigned fuel);
#endif
