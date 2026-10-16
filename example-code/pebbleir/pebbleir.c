#include "pebbleir.h"
#include <ctype.h>
#include <limits.h>
#include <string.h>

static bool fail(char error[PIR_ERROR], const char *message)
{
    (void)snprintf(error, PIR_ERROR, "%s", message);
    return false;
}

void pir_init(Pir *f) { memset(f, 0, sizeof *f); }

unsigned pir_block(Pir *f)
{
    if (f->blocks >= PIR_BLOCKS || f->revision == ULONG_MAX)
        return PIR_BLOCKS;
    ++f->revision;
    return f->blocks++;
}

unsigned pir_value(Pir *f, unsigned block, PirOp op, PirType type,
                   unsigned a, unsigned b, unsigned imm,
                   unsigned p, unsigned q)
{
    if (block >= f->blocks || block >= PIR_BLOCKS || f->count >= PIR_VALUES ||
        f->terms[block].op != PIR_UNSET || f->revision == ULONG_MAX)
        return 0;
    unsigned id = ++f->count;
    f->values[id] = (PirValue){op, type, block, a, b, imm, p, q};
    ++f->revision;
    return id;
}

bool pir_term(Pir *f, unsigned block, PirTerm term)
{
    if (block >= f->blocks || block >= PIR_BLOCKS || f->revision == ULONG_MAX)
        return false;
    f->terms[block] = term;
    ++f->revision;
    return true;
}

static unsigned bit(unsigned b) { return 1U << b; }
static bool live(const Pir *f, unsigned id)
{
    return id > 0 && id <= f->count && f->values[id].op != PIR_DEAD;
}

static bool available(const Pir *f, const PirFacts *g, unsigned id,
                      unsigned block, unsigned before)
{
    if (!live(f, id)) return false;
    unsigned def = f->values[id].block;
    return def == block ? id < before : (g->dom[block] & bit(def)) != 0;
}

bool pir_verify(const Pir *f, PirFacts *facts, char error[PIR_ERROR])
{
    PirFacts g = {{0}, {0}, 0, NULL};
    if (!f->blocks || f->blocks > PIR_BLOCKS || f->count > PIR_VALUES)
        return fail(error, "capacity or empty function");
    unsigned reach = 1, inputs = 0;
    for (unsigned b = 0; b < f->blocks; ++b) {
        PirTerm t = f->terms[b];
        if (t.op < PIR_RET || t.op > PIR_BRANCH)
            return fail(error, "missing or unknown terminator");
        if (t.op != PIR_RET) {
            if (t.x >= f->blocks || t.x == 0)
                return fail(error, "invalid target or edge into entry");
            g.pred[t.x] |= bit(b);
        }
        if (t.op == PIR_BRANCH) {
            if (t.y >= f->blocks || t.y == 0 || t.x == t.y)
                return fail(error, "invalid or duplicate branch target");
            g.pred[t.y] |= bit(b);
        }
        if ((t.op == PIR_RET && (t.x || t.y)) ||
            (t.op == PIR_JUMP && (t.value || t.y)))
            return fail(error, "noncanonical terminator fields");
    }
    unsigned old;
    do {
        old = reach;
        for (unsigned b = 0; b < f->blocks; ++b)
            if (g.pred[b] & reach) reach |= bit(b);
    } while (old != reach);
    unsigned all = (1U << f->blocks) - 1U;
    if (reach != all) return fail(error, "unreachable block");
    g.dom[0] = 1;
    for (unsigned b = 1; b < f->blocks; ++b) g.dom[b] = all;
    bool changed;
    do {
        changed = false;
        for (unsigned b = 1; b < f->blocks; ++b) {
            unsigned d = all;
            for (unsigned p = 0; p < f->blocks; ++p)
                if (g.pred[b] & bit(p)) d &= g.dom[p];
            d |= bit(b);
            changed |= d != g.dom[b];
            g.dom[b] = d;
        }
    } while (changed);
    bool ordinary[PIR_BLOCKS] = {false};
    /* Validate every definition's shape before following any operand. */
    for (unsigned i = 1; i <= f->count; ++i) {
        PirValue v = f->values[i];
        if (v.op < PIR_DEAD || v.op > PIR_PHI ||
            v.type < PIR_U8 || v.type > PIR_I1 || v.block >= f->blocks)
            return fail(error, "invalid value header");
        if (v.op == PIR_DEAD) {
            if (v.type || v.block || v.a || v.b || v.imm || v.p || v.q)
                return fail(error, "noncanonical tombstone");
            continue;
        }
        if (v.op == PIR_PHI) {
            if (ordinary[v.block] || v.block == 0 || v.p >= f->blocks ||
                v.q >= f->blocks || v.p == v.q || v.imm ||
                g.pred[v.block] != (bit(v.p) | bit(v.q)))
                return fail(error, "phi prefix or predecessor mismatch");
        } else {
            ordinary[v.block] = true;
            if (v.p || v.q) return fail(error, "unexpected predecessor fields");
        }
        if (v.op == PIR_INPUT) {
            if (v.block || v.type != PIR_U8 || ++inputs > 1 ||
                v.a || v.b || v.imm)
                return fail(error, "invalid input");
        } else if (v.op == PIR_CONST) {
            if (v.a || v.b || v.imm > (v.type == PIR_U8 ? 255U : 1U))
                return fail(error, "invalid constant");
        } else if (v.op != PIR_PHI) {
            if (v.imm || v.type != (v.op == PIR_LT ? PIR_I1 : PIR_U8))
                return fail(error, "invalid operation result type");
        }
    }
    for (unsigned i = 1; i <= f->count; ++i) {
        PirValue v = f->values[i];
        if (v.op < PIR_ADD) continue;
        unsigned ba = v.op == PIR_PHI ? v.p : v.block;
        unsigned bb = v.op == PIR_PHI ? v.q : v.block;
        unsigned before = v.op == PIR_PHI ? f->count + 1 : i;
        if (!available(f, &g, v.a, ba, before) ||
            !available(f, &g, v.b, bb, before))
            return fail(error, "operand does not dominate its use");
        PirType ty = v.op == PIR_PHI ? v.type : PIR_U8;
        if (f->values[v.a].type != ty || f->values[v.b].type != ty)
            return fail(error, "operand type mismatch");
    }
    for (unsigned b = 0; b < f->blocks; ++b) {
        PirTerm t = f->terms[b];
        if (t.op == PIR_JUMP) continue;
        if (!available(f, &g, t.value, b, f->count + 1))
            return fail(error, "terminator operand unavailable");
        if (f->values[t.value].type != (t.op == PIR_RET ? PIR_U8 : PIR_I1))
            return fail(error, "terminator operand type mismatch");
    }
    g.revision = f->revision;
    g.owner = f;
    *facts = g;
    error[0] = '\0';
    return true;
}

PirResult pir_run(const Pir *f, unsigned input, unsigned fuel)
{
    PirFacts facts;
    char error[PIR_ERROR];
    PirResult r = {PIR_INVALID, 0, 0};
    if (input > 255 || !pir_verify(f, &facts, error)) return r;
    unsigned reg[PIR_VALUES + 1] = {0}, temp[PIR_VALUES + 1] = {0};
    unsigned block = 0, pred = PIR_BLOCKS;
    while (r.steps < fuel) {
        ++r.steps;
        /* Capture the entire phi bundle before writing any destination. */
        for (unsigned i = 1; i <= f->count; ++i) {
            PirValue v = f->values[i];
            if (v.block == block && v.op == PIR_PHI)
                temp[i] = reg[pred == v.p ? v.a : v.b];
        }
        for (unsigned i = 1; i <= f->count; ++i) {
            PirValue v = f->values[i];
            if (v.block != block) continue;
            switch (v.op) {
            case PIR_DEAD: break;
            case PIR_INPUT: reg[i] = input; break;
            case PIR_CONST: reg[i] = v.imm; break;
            case PIR_ADD: reg[i] = (reg[v.a] + reg[v.b]) & 255U; break;
            case PIR_MUL: reg[i] = (reg[v.a] * reg[v.b]) & 255U; break;
            case PIR_LT: reg[i] = reg[v.a] < reg[v.b]; break;
            case PIR_PHI: reg[i] = temp[i]; break;
            }
        }
        PirTerm t = f->terms[block];
        if (t.op == PIR_RET) {
            r.status = PIR_OK; r.value = reg[t.value]; return r;
        }
        pred = block;
        block = t.op == PIR_JUMP || reg[t.value] ? t.x : t.y;
    }
    r.status = PIR_FUEL;
    return r;
}

static const char *const names[] = {"dead", "input", "const", "add", "mul", "lt", "phi"};
static const char *const terms[] = {"unset", "ret", "jump", "branch"};

bool pir_print(FILE *stream, const Pir *f)
{
    PirFacts facts; char error[PIR_ERROR];
    if (!pir_verify(f, &facts, error)) return false;
    if (fprintf(stream, "pebble 1 %u %u\n", f->blocks, f->count) < 0) return false;
    for (unsigned i = 1; i <= f->count; ++i) {
        PirValue v = f->values[i];
        if (fprintf(stream, "v %u %u %s %s %u %u %u %u %u\n", i,
            v.block, names[v.op], v.type == PIR_U8 ? "u8" : "i1",
            v.a, v.b, v.imm, v.p, v.q) < 0) return false;
    }
    for (unsigned b = 0; b < f->blocks; ++b) {
        PirTerm t = f->terms[b];
        if (fprintf(stream, "t %u %s %u %u %u\n", b, terms[t.op],
                    t.value, t.x, t.y) < 0) return false;
    }
    return fputs("end\n", stream) >= 0 && !ferror(stream);
}

static unsigned tokens(FILE *stream, char line[256], char *word[12])
{
    if (!fgets(line, 256, stream)) return 0;
    size_t n = strlen(line);
    if (!n || line[n - 1] != '\n') return 0;
    unsigned count = 0; char *p = line;
    while (*p) {
        while (isspace((unsigned char)*p)) ++p;
        if (!*p) break;
        if (count == 12) return 0;
        word[count++] = p;
        while (*p && !isspace((unsigned char)*p)) ++p;
        if (*p) *p++ = '\0';
    }
    return count;
}

static bool number(const char *s, unsigned *out)
{
    unsigned n = 0;
    if (!*s) return false;
    for (; *s; ++s) {
        if (*s < '0' || *s > '9') return false;
        unsigned d = (unsigned)(*s - '0');
        if (n > (UINT_MAX - d) / 10U) return false;
        n = n * 10U + d;
    }
    *out = n; return true;
}

bool pir_parse(FILE *stream, Pir *out, char error[PIR_ERROR])
{
    Pir f; pir_init(&f);
    char line[256], *w[12];
    if (tokens(stream, line, w) != 4 || strcmp(w[0], "pebble") ||
        strcmp(w[1], "1") || !number(w[2], &f.blocks) ||
        !number(w[3], &f.count) || f.blocks > PIR_BLOCKS || f.count > PIR_VALUES)
        return fail(error, "bad text header");
    for (unsigned i = 1; i <= f.count; ++i) {
        unsigned id, op = 0; PirValue *v = &f.values[i];
        if (tokens(stream, line, w) != 10 || strcmp(w[0], "v") ||
            !number(w[1], &id) || id != i || !number(w[2], &v->block))
            return fail(error, "bad value record");
        while (op <= PIR_PHI && strcmp(w[3], names[op])) ++op;
        if (op > PIR_PHI || (strcmp(w[4], "u8") && strcmp(w[4], "i1")))
            return fail(error, "unknown opcode or type");
        v->op = (PirOp)op; v->type = !strcmp(w[4], "i1") ? PIR_I1 : PIR_U8;
        if (!number(w[5], &v->a) || !number(w[6], &v->b) ||
            !number(w[7], &v->imm) || !number(w[8], &v->p) || !number(w[9], &v->q))
            return fail(error, "bad value field");
    }
    for (unsigned b = 0; b < f.blocks; ++b) {
        unsigned id, op = 1; PirTerm *t = &f.terms[b];
        if (tokens(stream, line, w) != 6 || strcmp(w[0], "t") ||
            !number(w[1], &id) || id != b) return fail(error, "bad terminator record");
        while (op <= PIR_BRANCH && strcmp(w[2], terms[op])) ++op;
        if (op > PIR_BRANCH || !number(w[3], &t->value) ||
            !number(w[4], &t->x) || !number(w[5], &t->y))
            return fail(error, "bad terminator field");
        t->op = (PirTermOp)op;
    }
    if (tokens(stream, line, w) != 1 || strcmp(w[0], "end") ||
        fgetc(stream) != EOF || ferror(stream)) return fail(error, "bad text end");
    PirFacts facts;
    if (!pir_verify(&f, &facts, error)) return false;
    *out = f; return true;
}

typedef struct { const char *p; Pir f; unsigned input, depth; bool ok; } Parser;
static void spaces(Parser *p) { while (isspace((unsigned char)*p->p)) ++p->p; }
static unsigned sum(Parser *p);
static unsigned atom(Parser *p)
{
    spaces(p);
    if (++p->depth > 32) { p->ok = false; --p->depth; return 0; }
    unsigned id = 0;
    if (*p->p == 'x') { ++p->p; id = p->input; }
    else if (*p->p == '(') {
        ++p->p; id = sum(p); spaces(p);
        if (*p->p != ')') p->ok = false; else ++p->p;
    } else if (*p->p >= '0' && *p->p <= '9') {
        unsigned n = 0;
        while (*p->p >= '0' && *p->p <= '9') {
            unsigned d = (unsigned)(*p->p++ - '0');
            if (n > (255U - d) / 10U) { p->ok = false; break; }
            n = n * 10U + d;
        }
        id = pir_value(&p->f, 0, PIR_CONST, PIR_U8, 0, 0, n, 0, 0);
    } else p->ok = false;
    --p->depth;
    if (!id) p->ok = false;
    return id;
}
static unsigned product(Parser *p)
{
    unsigned left = atom(p); spaces(p);
    while (p->ok && *p->p == '*') {
        ++p->p; unsigned right = atom(p);
        left = pir_value(&p->f, 0, PIR_MUL, PIR_U8, left, right, 0, 0, 0);
        if (!left) p->ok = false;
        spaces(p);
    }
    return left;
}
static unsigned sum(Parser *p)
{
    unsigned left = product(p); spaces(p);
    while (p->ok && *p->p == '+') {
        ++p->p; unsigned right = product(p);
        left = pir_value(&p->f, 0, PIR_ADD, PIR_U8, left, right, 0, 0, 0);
        if (!left) p->ok = false;
        spaces(p);
    }
    return left;
}
bool pir_compile(const char *expression, Pir *out, char error[PIR_ERROR])
{
    if (strlen(expression) > 1024) return fail(error, "source too long");
    Parser p; memset(&p, 0, sizeof p); p.p = expression; p.ok = true;
    (void)pir_block(&p.f);
    p.input = pir_value(&p.f, 0, PIR_INPUT, PIR_U8, 0, 0, 0, 0, 0);
    unsigned result = sum(&p); spaces(&p);
    if (!p.ok || *p.p || !pir_term(&p.f, 0, (PirTerm){PIR_RET, result, 0, 0}))
        return fail(error, "invalid expression or source capacity exceeded");
    PirFacts facts;
    if (!pir_verify(&p.f, &facts, error)) return false;
    *out = p.f; return true;
}

bool pir_replace(Pir *f, unsigned from, unsigned to, char error[PIR_ERROR])
{
    PirFacts facts;
    if (!pir_verify(f, &facts, error)) return false;
    if (!live(f, from) || !live(f, to) ||
        f->values[from].type != f->values[to].type || f->revision == ULONG_MAX)
        return fail(error, "invalid replacement");
    Pir next = *f;
    for (unsigned i = 1; i <= next.count; ++i) {
        PirValue *v = &next.values[i];
        if (v->op < PIR_ADD) continue;
        if (v->a == from) v->a = to;
        if (v->b == from) v->b = to;
    }
    for (unsigned b = 0; b < next.blocks; ++b)
        if (next.terms[b].value == from) next.terms[b].value = to;
    ++next.revision;
    if (!pir_verify(&next, &facts, error)) return false;
    *f = next; return true;
}

bool pir_optimize(Pir *f, char error[PIR_ERROR])
{
    PirFacts facts;
    if (!pir_verify(f, &facts, error)) return false;
    if (f->revision == ULONG_MAX) return fail(error, "revision exhausted");
    Pir next = *f; bool changed;
    do {
        changed = false;
        for (unsigned i = 1; i <= next.count; ++i) {
            PirValue *v = &next.values[i];
            if (v->op != PIR_ADD && v->op != PIR_MUL && v->op != PIR_LT) continue;
            PirValue a = next.values[v->a], b = next.values[v->b];
            if (a.op != PIR_CONST || b.op != PIR_CONST) continue;
            unsigned n = v->op == PIR_ADD ? (a.imm + b.imm) & 255U :
                         v->op == PIR_MUL ? (a.imm * b.imm) & 255U : a.imm < b.imm;
            *v = (PirValue){PIR_CONST, v->type, v->block, 0, 0, n, 0, 0};
            changed = true;
        }
    } while (changed);
    bool mark[PIR_VALUES + 1] = {false};
    for (unsigned b = 0; b < next.blocks; ++b)
        if (next.terms[b].op != PIR_JUMP) mark[next.terms[b].value] = true;
    do {
        changed = false;
        for (unsigned i = 1; i <= next.count; ++i) if (mark[i]) {
            PirValue v = next.values[i];
            if (v.op < PIR_ADD) continue;
            if (!mark[v.a]) { mark[v.a] = true; changed = true; }
            if (!mark[v.b]) { mark[v.b] = true; changed = true; }
        }
    } while (changed);
    for (unsigned i = 1; i <= next.count; ++i)
        if (!mark[i]) next.values[i] = (PirValue){0};
    ++next.revision;
    if (!pir_verify(&next, &facts, error)) return false;
    *f = next; return true;
}

static bool emit(PirVm *vm, PirVmOp op, unsigned dst, unsigned a, unsigned b)
{
    if (vm->count == PIR_CODE) return false;
    vm->code[vm->count++] = (PirInsn){op, dst, a, b};
    return true;
}
static bool edge(const Pir *f, PirVm *vm, unsigned from, unsigned to)
{
    for (unsigned i = 1; i <= f->count; ++i) {
        PirValue v = f->values[i];
        if (v.op == PIR_PHI && v.block == to &&
            !emit(vm, VM_MOVE, PIR_VALUES + i, from == v.p ? v.a : v.b, 0))
            return false;
    }
    for (unsigned i = 1; i <= f->count; ++i) {
        PirValue v = f->values[i];
        if (v.op == PIR_PHI && v.block == to &&
            !emit(vm, VM_MOVE, i, PIR_VALUES + i, 0)) return false;
    }
    /* Jump operands hold block numbers until final fixup. */
    return emit(vm, VM_JUMP, 0, to, 0);
}
bool pir_lower(const Pir *f, PirVm *out, char error[PIR_ERROR])
{
    PirFacts facts;
    if (!pir_verify(f, &facts, error)) return false;
    PirVm vm = {0}; unsigned label[PIR_BLOCKS] = {0};
    for (unsigned b = 0; b < f->blocks; ++b) {
        label[b] = vm.count;
        for (unsigned i = 1; i <= f->count; ++i) {
            PirValue v = f->values[i];
            if (v.block != b || v.op == PIR_DEAD || v.op == PIR_PHI) continue;
            PirVmOp op = v.op == PIR_INPUT ? VM_INPUT : v.op == PIR_CONST ? VM_CONST :
                         v.op == PIR_ADD ? VM_ADD : v.op == PIR_MUL ? VM_MUL : VM_LT;
            if (!emit(&vm, op, i, v.op == PIR_CONST ? v.imm : v.a, v.b))
                return fail(error, "VM capacity exceeded");
        }
        PirTerm t = f->terms[b];
        if (t.op == PIR_RET) {
            if (!emit(&vm, VM_RET, 0, t.value, 0)) return fail(error, "VM capacity exceeded");
        } else if (t.op == PIR_JUMP) {
            if (!edge(f, &vm, b, t.x)) return fail(error, "VM capacity exceeded");
        } else {
            unsigned at = vm.count;
            if (!emit(&vm, VM_BRANCH, t.value, at + 1, 0) || !edge(f, &vm, b, t.x))
                return fail(error, "VM capacity exceeded");
            vm.code[at].b = vm.count;
            if (!edge(f, &vm, b, t.y)) return fail(error, "VM capacity exceeded");
        }
    }
    for (unsigned i = 0; i < vm.count; ++i)
        if (vm.code[i].op == VM_JUMP) vm.code[i].a = label[vm.code[i].a];
    *out = vm; return true;
}

PirResult pir_vm_run(const PirVm *vm, unsigned input, unsigned fuel)
{
    PirResult r = {PIR_INVALID, 0, 0};
    unsigned reg[2 * PIR_VALUES + 1] = {0}, pc = 0;
    if (input > 255 || !vm->count || vm->count > PIR_CODE) return r;
    while (r.steps < fuel) {
        if (pc >= vm->count) return r;
        PirInsn v = vm->code[pc++]; ++r.steps;
        /* All numeric fields fit both bounded register/PC checks below.
           Check by opcode so a constant is not mistaken for a register. */
        if (v.op < VM_INPUT || v.op > VM_RET) return r;
        if (v.op <= VM_MOVE && (!v.dst || v.dst > 2 * PIR_VALUES)) return r;
        if ((v.op == VM_MOVE || v.op == VM_RET ||
             v.op == VM_ADD || v.op == VM_MUL || v.op == VM_LT) &&
            (!v.a || v.a > 2 * PIR_VALUES)) return r;
        if ((v.op == VM_ADD || v.op == VM_MUL || v.op == VM_LT) &&
            (!v.b || v.b > 2 * PIR_VALUES)) return r;
        if (v.op == VM_CONST && v.a > 255) return r;
        if (v.op == VM_JUMP && v.a >= vm->count) return r;
        if (v.op == VM_BRANCH && (!v.dst || v.dst > 2 * PIR_VALUES ||
            v.a >= vm->count || v.b >= vm->count || reg[v.dst] > 1)) return r;
        switch (v.op) {
        case VM_INPUT: reg[v.dst] = input; break;
        case VM_CONST: reg[v.dst] = v.a; break;
        case VM_ADD: reg[v.dst] = (reg[v.a] + reg[v.b]) & 255U; break;
        case VM_MUL: reg[v.dst] = (reg[v.a] * reg[v.b]) & 255U; break;
        case VM_LT: reg[v.dst] = reg[v.a] < reg[v.b]; break;
        case VM_MOVE: reg[v.dst] = reg[v.a]; break;
        case VM_JUMP: pc = v.a; break;
        case VM_BRANCH: pc = reg[v.dst] ? v.a : v.b; break;
        case VM_RET: r.status = PIR_OK; r.value = reg[v.a]; return r;
        }
    }
    r.status = PIR_FUEL; return r;
}
