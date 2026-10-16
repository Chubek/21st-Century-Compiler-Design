/* C11, Chapters 1--16 of the companion. Assertions must remain enabled. */
#include "pebbleir.h"
#include <assert.h>
#include <limits.h>
#include <stdlib.h>
#include <string.h>

static unsigned val(Pir *f, unsigned b, PirOp op, PirType ty,
                    unsigned a, unsigned c, unsigned imm)
{
    unsigned id = pir_value(f, b, op, ty, a, c, imm, 0, 0);
    assert(id); return id;
}
static void term(Pir *f, unsigned b, PirTermOp op, unsigned v, unsigned x, unsigned y)
{
    assert(pir_term(f, b, (PirTerm){op, v, x, y}));
}
static void roundtrip(const Pir *f)
{
    FILE *a = tmpfile(), *b = tmpfile(); assert(a && b);
    Pir parsed; char error[PIR_ERROR];
    assert(pir_print(a, f)); rewind(a);
    assert(pir_parse(a, &parsed, error));
    assert(pir_print(b, &parsed)); rewind(a); rewind(b);
    int x, y;
    do { x = fgetc(a); y = fgetc(b); assert(x == y); } while (x != EOF);
    assert(!ferror(a) && !ferror(b));
    assert(fclose(a) == 0 && fclose(b) == 0);
}

static unsigned sum_oracle(unsigned x)
{
    unsigned s = 0;
    for (unsigned i = 0; i < x; ++i) s = (s + i) & 255U;
    return s;
}
static unsigned diamond_oracle(unsigned x)
{
    return x < 10 ? (x + 1) & 255U : (x * 2) & 255U;
}
static unsigned swap_oracle(unsigned x) { (void)x; return 1; }

static void check_function(Pir *f, unsigned (*oracle)(unsigned))
{
    PirFacts facts; char error[PIR_ERROR];
    assert(pir_verify(f, &facts, error));
    assert(facts.owner == f && facts.revision == f->revision);
    roundtrip(f);
    Pir optimized = *f; PirVm vm, optimized_vm;
    assert(pir_lower(f, &vm, error));
    assert(pir_optimize(&optimized, error));
    assert(pir_lower(&optimized, &optimized_vm, error));
    roundtrip(&optimized);
    for (unsigned x = 0; x < 256; ++x) {
        PirResult a = pir_run(f, x, 10000), b = pir_run(&optimized, x, 10000);
        PirResult c = pir_vm_run(&vm, x, 100000), d = pir_vm_run(&optimized_vm, x, 100000);
        assert(a.status == PIR_OK && b.status == PIR_OK);
        assert(c.status == PIR_OK && d.status == PIR_OK);
        assert(a.value == oracle(x) && b.value == a.value);
        assert(c.value == a.value && d.value == a.value);
    }
}

static Pir diamond(void)
{
    Pir f; pir_init(&f);
    for (unsigned b = 0; b < 4; ++b) assert(pir_block(&f) == b);
    unsigned x = val(&f, 0, PIR_INPUT, PIR_U8, 0, 0, 0);
    unsigned ten = val(&f, 0, PIR_CONST, PIR_U8, 0, 0, 10);
    unsigned c = val(&f, 0, PIR_LT, PIR_I1, x, ten, 0);
    term(&f, 0, PIR_BRANCH, c, 1, 2);
    unsigned one = val(&f, 1, PIR_CONST, PIR_U8, 0, 0, 1);
    unsigned left = val(&f, 1, PIR_ADD, PIR_U8, x, one, 0);
    term(&f, 1, PIR_JUMP, 0, 3, 0);
    unsigned two = val(&f, 2, PIR_CONST, PIR_U8, 0, 0, 2);
    unsigned right = val(&f, 2, PIR_MUL, PIR_U8, x, two, 0);
    term(&f, 2, PIR_JUMP, 0, 3, 0);
    unsigned phi = pir_value(&f, 3, PIR_PHI, PIR_U8, left, right, 0, 1, 2);
    assert(phi); term(&f, 3, PIR_RET, phi, 0, 0);
    return f;
}

static Pir loop(bool swap)
{
    Pir f; pir_init(&f);
    for (unsigned b = 0; b < 4; ++b) assert(pir_block(&f) == b);
    unsigned x = val(&f, 0, PIR_INPUT, PIR_U8, 0, 0, 0);
    unsigned zero = val(&f, 0, PIR_CONST, PIR_U8, 0, 0, 0);
    unsigned one = val(&f, 0, PIR_CONST, PIR_U8, 0, 0, 1);
    unsigned two = val(&f, 0, PIR_CONST, PIR_U8, 0, 0, 2);
    term(&f, 0, PIR_JUMP, 0, 1, 0);
    unsigned i = pir_value(&f, 1, PIR_PHI, PIR_U8, zero, 0, 0, 0, 2);
    unsigned s = pir_value(&f, 1, PIR_PHI, PIR_U8, swap ? one : zero, 0, 0, 0, 2);
    unsigned t = swap ? pir_value(&f, 1, PIR_PHI, PIR_U8, two, s, 0, 0, 2) : 0;
    unsigned c = val(&f, 1, PIR_LT, PIR_I1, i, swap ? two : x, 0);
    term(&f, 1, PIR_BRANCH, c, 2, 3);
    unsigned ni = val(&f, 2, PIR_ADD, PIR_U8, i, one, 0);
    unsigned ns = swap ? t : val(&f, 2, PIR_ADD, PIR_U8, s, i, 0);
    term(&f, 2, PIR_JUMP, 0, 1, 0);
    /* Construction-only backpatch: no facts have been published yet. */
    f.values[i].b = ni; f.values[s].b = ns; ++f.revision;
    term(&f, 3, PIR_RET, s, 0, 0);
    return f;
}

enum { TREE = 31 };
typedef struct { unsigned op, literal, left, right; } Node;
static unsigned random_state = 17;
static unsigned random_byte(void)
{
    random_state = (random_state * 109U + 89U) & 65535U;
    return random_state & 255U;
}
static unsigned make_tree(Node nodes[TREE], unsigned *count, unsigned depth)
{
    unsigned id = (*count)++; assert(id < TREE);
    Node *n = &nodes[id]; n->op = depth ? random_byte() % 4 : random_byte() % 2;
    n->literal = random_byte();
    if (n->op >= 2) {
        n->left = make_tree(nodes, count, depth - 1);
        n->right = make_tree(nodes, count, depth - 1);
    }
    return id;
}
static unsigned tree_eval(const Node nodes[TREE], unsigned id, unsigned x)
{
    Node n = nodes[id];
    if (n.op == 0) return x;
    if (n.op == 1) return n.literal;
    unsigned a = tree_eval(nodes, n.left, x), b = tree_eval(nodes, n.right, x);
    return (n.op == 2 ? a + b : a * b) & 255U;
}
static void tree_text(const Node nodes[TREE], unsigned id, char *s, size_t *used)
{
    Node n = nodes[id];
    if (n.op < 2) {
        int k = n.op == 0 ? snprintf(s + *used, 1024 - *used, "x") :
                          snprintf(s + *used, 1024 - *used, "%u", n.literal);
        assert(k > 0 && (size_t)k < 1024 - *used); *used += (size_t)k;
    } else {
        s[(*used)++] = '('; tree_text(nodes, n.left, s, used);
        s[(*used)++] = n.op == 2 ? '+' : '*'; tree_text(nodes, n.right, s, used);
        s[(*used)++] = ')'; s[*used] = '\0';
    }
}
static void generated(void)
{
    for (unsigned seed = 0; seed < 128; ++seed) {
        Node nodes[TREE] = {{0}}; unsigned count = 0; size_t used = 0;
        random_state = seed + 17;
        unsigned root = make_tree(nodes, &count, 3);
        char text[1024] = {0}, error[PIR_ERROR]; tree_text(nodes, root, text, &used);
        Pir f; assert(pir_compile(text, &f, error));
        Pir original = f; assert(pir_optimize(&f, error));
        PirVm vm; assert(pir_lower(&f, &vm, error));
        for (unsigned x = 0; x < 256; ++x) {
            unsigned expected = tree_eval(nodes, root, x);
            PirResult a = pir_run(&original, x, 4), b = pir_run(&f, x, 4);
            PirResult c = pir_vm_run(&vm, x, 1000);
            assert(a.status == PIR_OK && b.status == PIR_OK && c.status == PIR_OK);
            assert(a.value == expected && b.value == expected && c.value == expected);
        }
    }
}

static void malformed(void)
{
    char error[PIR_ERROR]; PirFacts facts; Pir f = diamond(), saved;
    saved = f; f.values[5].a = 7; /* sibling branch does not dominate */
    assert(!pir_verify(&f, &facts, error)); f = saved;
    f.values[8].p = 0; assert(!pir_verify(&f, &facts, error)); f = saved;
    f.terms[0].value = 1; assert(!pir_verify(&f, &facts, error)); f = saved;
    f.values[2].imm = 256; assert(!pir_verify(&f, &facts, error)); f = saved;
    f.terms[2].x = PIR_BLOCKS; assert(!pir_verify(&f, &facts, error)); f = saved;
    f.values[1].block = PIR_BLOCKS; assert(!pir_verify(&f, &facts, error));
    assert(pir_run(&f, 0, 100).status == PIR_INVALID);
    f = saved; f.terms[0] = (PirTerm){PIR_JUMP, 0, 3, 0};
    assert(!pir_verify(&f, &facts, error)); /* unreachable arms */
    f = saved; assert(!pir_replace(&f, 1, 5, error));
    assert(memcmp(&f, &saved, sizeof f) == 0);
    const char *bad[] = {"", "x+", "256", "x x", "(x", "x)", "-1", "1/0"};
    for (unsigned i = 0; i < sizeof bad / sizeof bad[0]; ++i) {
        assert(!pir_compile(bad[i], &f, error));
        assert(memcmp(&f, &saved, sizeof f) == 0);
    }
    FILE *stream = tmpfile(); assert(stream);
    fputs("pebble 1 1 999999999999999999999999999999\n", stream); rewind(stream);
    assert(!pir_parse(stream, &f, error)); assert(memcmp(&f, &saved, sizeof f) == 0);
    assert(fclose(stream) == 0);
    assert(pir_value(&f, 0, PIR_CONST, PIR_U8, 0, 0, 3, 0, 0) == 0);
    f.revision = ULONG_MAX; saved = f;
    assert(!pir_optimize(&f, error) && memcmp(&f, &saved, sizeof f) == 0);
    pir_init(&f); assert(pir_block(&f) == 0);
    for (unsigned i = 0; i < PIR_VALUES; ++i) assert(pir_value(&f, 0, PIR_CONST, PIR_U8, 0, 0, 0, 0, 0));
    saved = f; assert(!pir_value(&f, 0, PIR_CONST, PIR_U8, 0, 0, 0, 0, 0));
    assert(memcmp(&f, &saved, sizeof f) == 0);
}

int main(void)
{
    Pir d = diamond(), l = loop(false), s = loop(true);
    check_function(&d, diamond_oracle); check_function(&l, sum_oracle);
    check_function(&s, swap_oracle); generated(); malformed();
    assert(pir_run(&l, 3, 8).status == PIR_FUEL);
    PirResult loop_result = pir_run(&l, 3, 9);
    assert(loop_result.status == PIR_OK && loop_result.value == 3 && loop_result.steps == 9);
    char error[PIR_ERROR]; Pir f; PirVm vm;
    assert(pir_compile("(x+7)*(3+5)", &f, error));
    assert(pir_lower(&f, &vm, error)); unsigned before = vm.count;
    PirFacts facts; assert(pir_verify(&f, &facts, error));
    assert(pir_optimize(&f, error)); assert(facts.revision != f.revision);
    assert(pir_lower(&f, &vm, error));
    assert(before == 8 && vm.count == 6);
    roundtrip(&f);
    assert(pir_vm_run(&vm, 9, 6).value == 128);
    assert(pir_vm_run(&vm, 9, 5).status == PIR_FUEL);
    assert(pir_run(&f, 9, 0).status == PIR_FUEL);
    vm.code[0].dst = 2 * PIR_VALUES + 1;
    assert(pir_vm_run(&vm, 9, 100).status == PIR_INVALID);
    const char *constant_cases[] = {"2+3*4", "(2+3)*4", "(2+3)*(4+5)"};
    const unsigned results[] = {14, 20, 45};
    for (unsigned i = 0; i < 3; ++i) {
        assert(pir_compile(constant_cases[i], &f, error));
        assert(pir_optimize(&f, error) && pir_lower(&f, &vm, error));
        assert(vm.count == 2);
        PirResult r = pir_vm_run(&vm, 9, 2);
        assert(r.status == PIR_OK && r.value == results[i]);
    }
    printf("PebbleIR: all checks passed; example VM instructions %u -> %u\n", before, 6U);
    return EXIT_SUCCESS;
}
