/* Chapter 1: pure, modulo-256 expressions. Requires C11. */
#ifndef CH01_EXPR_H
#define CH01_EXPR_H

typedef enum { LITERAL, INPUT, ADD, MUL } Kind;

typedef struct Expr {
    Kind kind;
    unsigned literal;
    struct Expr *left;
    struct Expr *right;
} Expr;

/* Inputs must be finite trees with no shared nodes or cycles.
   Literals and x must lie in 0..255; binary nodes need two children.
   Storage is caller-owned and must outlive every use of the tree. */
unsigned eval(const Expr *e, unsigned x);
void fold(Expr *e);

#endif
