/* Chapters 69--76: ABI differential tests. Linux/POSIX, C11, libffi, pthreads. */
#include "bookffi.h"
#include <assert.h>
#include <dlfcn.h>
#include <pthread.h>
#include <stdarg.h>
#include <stdio.h>
#include <string.h>

typedef struct { double weight; int32_t tag; } Pair;
typedef struct { uint64_t a, b, c; } Triple;
static Pair pair_step(Pair p, int32_t delta) { p.weight += 0.5; p.tag += delta; return p; }
static Triple triple_step(Triple p) { ++p.a; p.b += 2; p.c += 3; return p; }
static double variadic_sum(int count, ...)
{
    double sum = 0;
    va_list ap;
    va_start(ap, count);
    for (int i = 0; i < count; ++i) sum += va_arg(ap, double);
    va_end(ap);
    return sum;
}
static uint64_t eight(uint64_t a, uint64_t b, uint64_t c, uint64_t d,
                      uint64_t e, uint64_t f, uint64_t g, uint64_t h)
{ return a + 2*b + 3*c + 4*d + 5*e + 6*f + 7*g + 8*h; }
static double mixed(int32_t a, double b, int32_t c, double d)
{ return a + 2*b + 3*c + 4*d; }
static void *identity(void *p) { return p; }

static void scalar_tests(void)
{
    const int32_t cases[] = {INT32_MIN, -129, -1, 0, 1, 127, INT32_MAX};
    for (size_t i = 0; i < 7; ++i) for (size_t j = 0; j < 7; ++j) {
        int32_t direct = 99, foreign = 99;
        int64_t mathematical = (int64_t)cases[i] + cases[j];
        int expected = mathematical < INT32_MIN || mathematical > INT32_MAX ? BB_RANGE : BB_OK;
        assert(bb_add(cases[i], cases[j], &direct) == expected);
        assert(bb_add_via_ffi(cases[i], cases[j], &foreign) == expected);
        assert(direct == foreign);
        assert(direct == (expected == BB_OK ? (int32_t)mathematical : 99));
    }
    assert(bb_add_via_ffi(1, 2, NULL) == BB_INVALID);
    {
        FxPlan plan;
        FxKind types[8]; FxValue values[8], result;
        for (size_t i = 0; i < 8; ++i) { types[i] = FX_U64; values[i].u64 = i+1; }
        assert(fx_prepare(&plan, FX_U64, types, 8) == BB_OK);
        assert(fx_call(&plan, FFI_FN(eight), types, values, 8, &result) == BB_OK);
        assert(result.u64 == eight(1,2,3,4,5,6,7,8) && result.u64 == 204);
        result.u64 = 77;
        types[7] = FX_F64;
        assert(fx_call(&plan, FFI_FN(eight), types, values, 8, &result) == BB_INVALID);
        assert(result.u64 == 77);
        assert(fx_prepare(&plan, FX_U64, types, 9) == BB_INVALID);
        assert(fx_call(&plan, FFI_FN(eight), types, values, 8, &result) == BB_INVALID);
    }
    {
        FxPlan plan; FxValue result;
        FxKind types[] = {FX_I32, FX_F64, FX_I32, FX_F64};
        FxValue values[] = {{.i32=-3}, {.f64=1.5}, {.i32=7}, {.f64=2.25}};
        assert(fx_prepare(&plan, FX_F64, types, 4) == BB_OK);
        assert(fx_call(&plan, FFI_FN(mixed), types, values, 4, &result) == BB_OK);
        assert(result.f64 == mixed(-3,1.5,7,2.25) && result.f64 == 30);
    }
    {
        int target = 42;
        FxPlan plan; FxKind type = FX_POINTER;
        FxValue input = {.pointer=&target}, output;
        assert(fx_prepare(&plan, FX_POINTER, &type, 1) == BB_OK);
        assert(fx_call(&plan, FFI_FN(identity), &type, &input, 1, &output) == BB_OK);
        assert(output.pointer == &target);
    }
}

static void aggregate_tests(void)
{
    ffi_type *pair_fields[] = {&ffi_type_double, &ffi_type_sint32, NULL};
    ffi_type pair_type = {0, 0, FFI_TYPE_STRUCT, pair_fields};
    ffi_type *args[] = {&pair_type, &ffi_type_sint32};
    size_t offsets[2];
    ffi_cif cif;
    Pair p = {3.25, 8}, result;
    int32_t delta = 9;
    void *values[] = {&p, &delta};
    assert(ffi_get_struct_offsets(FFI_DEFAULT_ABI, &pair_type, offsets) == FFI_OK);
    assert(offsets[0] == offsetof(Pair, weight) && offsets[1] == offsetof(Pair, tag));
    assert(pair_type.size == sizeof(Pair) && pair_type.alignment == _Alignof(Pair));
    assert(ffi_prep_cif(&cif, FFI_DEFAULT_ABI, 2, &pair_type, args) == FFI_OK);
    ffi_call(&cif, FFI_FN(pair_step), &result, values);
    Pair expected = pair_step(p, delta);
    assert(result.weight == expected.weight && result.tag == expected.tag);
    {
        ffi_type *fields[] = {&ffi_type_uint64, &ffi_type_uint64, &ffi_type_uint64, NULL};
        ffi_type type = {0, 0, FFI_TYPE_STRUCT, fields};
        ffi_type *types[] = {&type};
        Triple t = {10,20,30}, r, want = triple_step(t);
        void *v[] = {&t};
        assert(ffi_prep_cif(&cif, FFI_DEFAULT_ABI, 1, &type, types) == FFI_OK);
        ffi_call(&cif, FFI_FN(triple_step), &r, v);
        assert(r.a == want.a && r.b == want.b && r.c == want.c);
    }
}

static void variadic_tests(void)
{
    ffi_cif cif;
    ffi_type *types[] = {&ffi_type_sint, &ffi_type_double, &ffi_type_double};
    int count = 2;
    double a = 1.25, b = 2.5, result = 0;
    void *values[] = {&count, &a, &b};
    assert(ffi_prep_cif_var(&cif, FFI_DEFAULT_ABI, 1, 3, &ffi_type_double, types) == FFI_OK);
    ffi_call(&cif, FFI_FN(variadic_sum), &result, values);
    assert(result == 3.75 && result == variadic_sum(2, a, b));
    types[1] = &ffi_type_float;
    assert(ffi_prep_cif_var(&cif, FFI_DEFAULT_ABI, 1, 3, &ffi_type_double, types) != FFI_OK);
}

static void closure_body(ffi_cif *cif, void *ret, void **args, void *context)
{
    int32_t index = *(int32_t *)args[0], bias = *(int32_t *)context;
    (void)cif;
    *(ffi_sarg *)ret = (ffi_sarg)index + bias;
}
static void closure_tests(void)
{
    ffi_cif cif;
    ffi_type *types[] = {&ffi_type_sint32};
    int32_t bias = 17;
    void *code = NULL;
    ffi_closure *closure = ffi_closure_alloc(sizeof(*closure), &code);
    int32_t (*function)(int32_t);
    assert(closure && code);
    assert(ffi_prep_cif(&cif, FFI_DEFAULT_ABI, 1, &ffi_type_sint32, types) == FFI_OK);
    assert(ffi_prep_closure_loc(closure, &cif, closure_body, &bias, code) == FFI_OK);
    /* Supported libffi/POSIX target represents code addresses at this width. */
    _Static_assert(sizeof(function) == sizeof(code), "function address width");
    memcpy(&function, &code, sizeof(function));
    for (int32_t i = -50; i <= 50; ++i) assert(function(i) == i+17);
    ffi_closure_free(closure); /* No retained pointer or active invocation. */
}

typedef struct { int calls, stop; } Counter;
static int count_callback(void *context, int32_t index, int32_t *out)
{
    Counter *c = context;
    ++c->calls;
    if (index == c->stop) return BB_CALLBACK;
    return bb_add_via_ffi(index, 2, out); /* Reentrant call into the native bridge. */
}
static void lifetime_tests(void)
{
    const unsigned char data[] = {65,0,255,66};
    BbBuffer *b = NULL;
    unsigned char output[4] = {1,2,3,4};
    assert(bb_new(data, sizeof(data), &b) == BB_OK);
    assert(bb_size(b) == 4);
    assert(bb_read(b, 0, 4, output) == BB_OK && !memcmp(data, output, 4));
    assert(bb_read(b, 4, 0, NULL) == BB_OK);
    assert(bb_read(b, SIZE_MAX, 1, output) == BB_RANGE);
    assert(bb_read(b, 1, SIZE_MAX, output) == BB_RANGE);
    assert(!memcmp(data, output, 4));
    bb_free(b); b = NULL;
    assert(bb_new(NULL, 0, &b) == BB_OK); bb_free(b); b = NULL;
    assert(bb_new(NULL, 1, &b) == BB_INVALID && b == NULL);
    assert(bb_new(data, BB_LIMIT+1, &b) == BB_RANGE && b == NULL);
    bb_free(NULL);
    for (int stop = -1; stop < 5; ++stop) {
        Counter c = {0, stop}; int64_t sum = -99;
        int status = bb_visit(5, count_callback, &c, &sum);
        assert(status == (stop < 0 ? BB_OK : BB_CALLBACK));
        assert(c.calls == (stop < 0 ? 5 : stop+1));
        assert(sum == (stop < 0 ? 20 : -99));
    }
}

static void *worker(void *unused)
{
    (void)unused;
    for (int32_t i = 0; i < 1000; ++i) {
        int32_t out;
        assert(bb_add_via_ffi(i, -i, &out) == BB_OK && out == 0);
    }
    return NULL;
}
static void plugin_test(const char *path)
{
    void *handle = dlopen(path, RTLD_NOW | RTLD_LOCAL);
    void *symbol;
    const BbApi *(*query)(uint32_t, size_t);
    const BbApi *api;
    int32_t sum;
    assert(handle);
    (void)dlerror(); symbol = dlsym(handle, "book_api_query");
    assert(dlerror() == NULL && symbol);
    _Static_assert(sizeof(query) == sizeof(symbol), "POSIX function address width");
    memcpy(&query, &symbol, sizeof(query));
    assert(query(2, sizeof(BbApi)) == NULL);
    assert(query(1, sizeof(BbApi)+1) == NULL);
    api = query(1, sizeof(BbApi));
    assert(api && api->version == 1 && api->size >= sizeof(BbApi) && api->add);
    assert(api->add(19,23,&sum) == BB_OK && sum == 42);
    assert(dlclose(handle) == 0); /* No retained table, objects, or calls. */
}
int main(int argc, char **argv)
{
    pthread_t threads[4];
    assert(argc == 2);
    scalar_tests(); aggregate_tests(); variadic_tests(); closure_tests(); lifetime_tests();
    for (size_t i = 0; i < 4; ++i) assert(!pthread_create(&threads[i], NULL, worker, NULL));
    for (size_t i = 0; i < 4; ++i) assert(!pthread_join(threads[i], NULL));
    plugin_test(argv[1]);
    puts("native ABI, ownership, callbacks, threads, and plugin checks passed");
    return 0;
}
