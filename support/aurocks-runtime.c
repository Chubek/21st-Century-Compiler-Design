/* Runtime template for aurocks.pl: emitted into each standalone C99 parser.
   Search nodes borrow C-stack storage; actions run only at complete acceptance. */
typedef enum { AU_OK, AU_SYNTAX, AU_LIMIT, AU_NOMEM, AU_INVALID } AUStatus;
typedef struct { size_t steps, depth; } AULimits;
typedef struct AUBlock AUBlock;
struct AUBlock { void *data; AUBlock *next; };
typedef struct {
    AUStatus status;
    void *value;
    size_t offset, steps;
    const char *expected;
    AUBlock *blocks;
} AUResult;
typedef struct {
    const char *input;
    size_t length, depth;
    AULimits limits;
    AUResult result;
} AUParser;
typedef struct AUNode AUNode;
struct AUNode {
    int rule; /* negative for a terminal */
    size_t begin, end, count;
    AUNode **child;
};
typedef struct AUCont AUCont;
struct AUCont {
    int (*call)(AUParser *, size_t, AUNode *, AUCont *);
    void *data;
};
typedef struct { int kind, out1, out2; unsigned int arg; } AURxState;
typedef struct {
    const AURxState *state;
    size_t count;
    int start;
    int (*class_test)(unsigned int, unsigned char);
    const char *name;
} AURx;
enum { AU_RX_MATCH, AU_RX_SPLIT, AU_RX_JMP, AU_RX_CHAR,
       AU_RX_CLASS, AU_RX_ANY, AU_RX_EOL };

/* AUROCKS_PROLOGUE */

static int au_tick(AUParser *p)
{
    if (p->result.status != AU_SYNTAX) return 0;
    if (p->result.steps == p->limits.steps) {
        p->result.status = AU_LIMIT;
        return 0;
    }
    ++p->result.steps;
    return 1;
}

static int au_enter(AUParser *p)
{
    if (!au_tick(p)) return 0;
    if (p->depth == p->limits.depth) {
        p->result.status = AU_LIMIT;
        return 0;
    }
    ++p->depth;
    return 1;
}

static void au_fail(AUParser *p, size_t at, const char *expected)
{
    if (!p->result.expected || at > p->result.offset) {
        p->result.offset = at;
        p->result.expected = expected;
    }
}

static void *au_alloc(AUParser *p, size_t n)
{
    AUBlock *b = (AUBlock *)malloc(sizeof *b);
    void *data = malloc(n ? n : 1);
    if (!b || !data) {
        free(b); free(data);
        p->result.status = AU_NOMEM;
        return NULL;
    }
    b->data = data;
    b->next = p->result.blocks;
    p->result.blocks = b;
    return data;
}

void au_result_free(AUResult *r)
{
    if (!r) return;
    while (r->blocks) {
        AUBlock *b = r->blocks;
        r->blocks = b->next;
        free(b->data); free(b);
    }
    r->value = NULL;
}

/* Mark before following epsilon edges: each state enters a closure once. */
static void au_closure(const AURxState *st, int q, int *list,
                       size_t *n, unsigned char *seen, int at_end)
{
    if (q < 0 || seen[q]) return;
    seen[q] = 1;
    switch (st[q].kind) {
    case AU_RX_SPLIT:
        au_closure(st, st[q].out1, list, n, seen, at_end);
        au_closure(st, st[q].out2, list, n, seen, at_end);
        break;
    case AU_RX_JMP:
        au_closure(st, st[q].out1, list, n, seen, at_end);
        break;
    case AU_RX_EOL:
        if (at_end) au_closure(st, st[q].out1, list, n, seen, at_end);
        break;
    default:
        list[(*n)++] = q;
        break;
    }
}

static size_t au_match(AUParser *p, const AURx *rx, size_t at)
{
    int current[rx->count], next[rx->count];
    unsigned char seen[rx->count];
    size_t count = 0, pos = at, best = at;
    memset(seen, 0, sizeof seen);
    au_closure(rx->state, rx->start, current, &count, seen, pos == p->length);
    for (;;) {
        if (!au_tick(p)) return at;
        for (size_t i = 0; i < count; ++i)
            if (rx->state[current[i]].kind == AU_RX_MATCH) best = pos;
        if (pos == p->length || !count) break;
        unsigned char ch = (unsigned char)p->input[pos];
        size_t n = 0;
        memset(seen, 0, sizeof seen);
        for (size_t i = 0; i < count; ++i) {
            const AURxState *s = &rx->state[current[i]];
            int matches = (s->kind == AU_RX_CHAR && s->arg == ch) ||
                (s->kind == AU_RX_ANY && ch != '\n') ||
                (s->kind == AU_RX_CLASS && rx->class_test(s->arg, ch));
            if (matches)
                au_closure(rx->state, s->out1, next, &n, seen, pos + 1 == p->length);
        }
        memcpy(current, next, n * sizeof *current);
        count = n;
        ++pos;
    }
    return best;
}

/* Generated tables and per-nonterminal wrappers are inserted here. */
/* AUROCKS_TABLES */

static size_t au_skip(AUParser *p, size_t at)
{
    if (au_layout < 0) return at;
    for (;;) {
        size_t end = au_match(p, &au_regex[au_layout], at);
        if (end == at || p->result.status != AU_SYNTAX) return at;
        at = end;
    }
}

static void *au_eval(AUParser *p, AUNode *node)
{
    if (node->rule < 0) {
        size_t n = node->end - node->begin;
        if (n == SIZE_MAX) { p->result.status = AU_NOMEM; return NULL; }
        char *text = (char *)au_alloc(p, n + 1);
        if (!text) return NULL;
        memcpy(text, p->input + node->begin, n);
        text[n] = 0;
        return text;
    }
    void *v[AU_MAX_RHS];
    char *token = NULL;
    size_t token_len = 0;
    for (size_t i = 0; i < node->count; ++i) {
        v[i] = au_eval(p, node->child[i]);
        if (p->result.status == AU_NOMEM) return NULL;
        if (node->child[i]->rule < 0) {
            token = (char *)v[i];
            token_len = node->child[i]->end - node->child[i]->begin;
        }
    }
    void *result = node->count == 1 ? v[0] : NULL;
    (void)v; (void)token; (void)token_len;
    switch (node->rule) {
/* AUROCKS_ACTIONS */
    }
    return result;
}

typedef struct {
    size_t rule, index, begin;
    AUNode **child;
    AUCont *outer;
} AUFrame;

static int au_seq(AUParser *, size_t, size_t, size_t, size_t, AUNode **, AUCont *);

static int au_resume(AUParser *p, size_t end, AUNode *node, AUCont *k)
{
    AUFrame *f = (AUFrame *)k->data;
    f->child[f->index] = node;
    return au_seq(p, f->rule, f->index + 1, f->begin, end, f->child, f->outer);
}

static int au_seq(AUParser *p, size_t rule, size_t index,
                   size_t begin, size_t at, AUNode **child, AUCont *outer)
{
    if (!au_enter(p)) return 0;
    int accepted = 0;
    const AURule *r = &au_rules[rule];
    if (index == r->count) {
        AUNode node = {(int)rule, begin, at, r->count, child};
        accepted = outer->call(p, at, &node, outer);
    } else {
        AUFrame frame = {rule, index, begin, child, outer};
        AUCont rest = {au_resume, &frame};
        int symbol = r->rhs[index];
        if (symbol >= 0) {
            accepted = au_nonterminal[symbol](p, at, &rest);
        } else {
            at = au_skip(p, at);
            const AURx *rx = &au_regex[-symbol - 1];
            size_t end = au_match(p, rx, at);
            if (p->result.status == AU_SYNTAX && end > at) {
                AUNode leaf = {-1, at, end, 0, NULL};
                accepted = au_resume(p, end, &leaf, &rest);
            } else {
                au_fail(p, at, rx->name);
            }
        }
    }
    --p->depth;
    return accepted;
}

static int au_choice(AUParser *p, size_t at, AUCont *k, size_t nt)
{
    if (!au_enter(p)) return 0;
    int accepted = 0;
    for (size_t r = au_first[nt]; r < au_first[nt + 1]; ++r) {
        AUNode *children[AU_MAX_RHS];
        if (au_seq(p, r, 0, at, at, children, k)) { accepted = 1; break; }
        if (p->result.status != AU_SYNTAX) break;
    }
    --p->depth;
    return accepted;
}

static int au_complete(AUParser *p, size_t end, AUNode *node, AUCont *k)
{
    (void)k;
    end = au_skip(p, end);
    if (p->result.status != AU_SYNTAX) return 0;
    if (end != p->length) { au_fail(p, end, "end of input"); return 0; }
    p->result.value = au_eval(p, node);
    if (p->result.status == AU_NOMEM) return 0;
    p->result.status = AU_OK;
    p->result.offset = end;
    p->result.expected = NULL;
    return 1;
}

/* AUROCKS_ENTRY */
