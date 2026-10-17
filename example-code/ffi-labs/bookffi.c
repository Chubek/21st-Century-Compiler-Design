/* Chapters 69--76: checked C11 core, scalar libffi plans, versioned API. */
#include "bookffi.h"
#include <stdlib.h>
#include <string.h>

struct BbBuffer { size_t size; unsigned char bytes[]; };

int bb_add(int32_t a, int32_t b, int32_t *out)
{
    int64_t sum = (int64_t)a + b;
    if (!out) return BB_INVALID;
    if (sum < INT32_MIN || sum > INT32_MAX) return BB_RANGE;
    *out = (int32_t)sum;
    return BB_OK;
}

int bb_new(const void *data, size_t size, BbBuffer **out)
{
    BbBuffer *b;
    if (!out || (!data && size)) return BB_INVALID;
    if (size > BB_LIMIT || size > SIZE_MAX - sizeof(*b)) return BB_RANGE;
    b = malloc(sizeof(*b) + size);
    if (!b) return BB_NOMEM;
    b->size = size;
    if (size) memcpy(b->bytes, data, size);
    *out = b;
    return BB_OK;
}

void bb_free(BbBuffer *b) { free(b); }
const unsigned char *bb_data(const BbBuffer *b) { return b->bytes; }
size_t bb_size(const BbBuffer *b) { return b->size; }

int bb_read(const BbBuffer *b, size_t offset, size_t count, void *out)
{
    if (!b || (!out && count)) return BB_INVALID;
    if (offset > b->size || count > b->size - offset) return BB_RANGE;
    if (count) memcpy(out, b->bytes + offset, count);
    return BB_OK;
}

int bb_visit(int32_t count, BbCallback callback, void *context, int64_t *out)
{
    int64_t sum = 0;
    if (!callback || !out) return BB_INVALID;
    if (count < 0 || count > BB_VISIT_LIMIT) return BB_RANGE;
    for (int32_t i = 0; i < count; ++i) {
        int32_t value = 0;
        if (callback(context, i, &value) != BB_OK) return BB_CALLBACK;
        sum += value; /* count bound keeps every int32_t sum within int64_t. */
    }
    *out = sum;
    return BB_OK;
}

static ffi_type *native_type(FxKind kind)
{
    switch (kind) {
    case FX_I32: return &ffi_type_sint32;
    case FX_U64: return &ffi_type_uint64;
    case FX_F64: return &ffi_type_double;
    case FX_POINTER: return &ffi_type_pointer;
    default: return NULL;
    }
}

int fx_prepare(FxPlan *p, FxKind result, const FxKind *kinds, size_t count)
{
    ffi_type *ret;
    if (!p) return BB_INVALID;
    p->ready = 0;
    ret = native_type(result);
    if (!ret || count > FX_MAX_ARGS || (!kinds && count)) return BB_INVALID;
    for (size_t i = 0; i < count; ++i) {
        p->types[i] = native_type(kinds[i]);
        if (!p->types[i]) return BB_INVALID;
        p->kinds[i] = kinds[i];
    }
    if (ffi_prep_cif(&p->cif, FFI_DEFAULT_ABI, (unsigned)count, ret, p->types) != FFI_OK)
        return BB_INVALID;
    p->count = count;
    p->result = result;
    p->ready = 1;
    return BB_OK;
}

int fx_call(FxPlan *p, void (*function)(void), const FxKind *kinds,
            FxValue *values, size_t count, FxValue *out)
{
    void *args[FX_MAX_ARGS];
    union { ffi_sarg signed_word; uint64_t u64; double f64; void *pointer; } result;
    void *return_storage;
    if (!p || !p->ready || !function || !out || count != p->count ||
        (count && (!kinds || !values))) return BB_INVALID;
    for (size_t i = 0; i < count; ++i) {
        if (kinds[i] != p->kinds[i]) return BB_INVALID;
        switch (kinds[i]) {
        case FX_I32: args[i] = &values[i].i32; break;
        case FX_U64: args[i] = &values[i].u64; break;
        case FX_F64: args[i] = &values[i].f64; break;
        case FX_POINTER: args[i] = &values[i].pointer; break;
        default: return BB_INVALID;
        }
    }
    switch (p->result) {
    case FX_I32: return_storage = &result.signed_word; break;
    case FX_U64: return_storage = &result.u64; break;
    case FX_F64: return_storage = &result.f64; break;
    case FX_POINTER: return_storage = &result.pointer; break;
    default: return BB_INVALID;
    }
    ffi_call(&p->cif, function, return_storage, args);
    switch (p->result) {
    case FX_I32: out->i32 = (int32_t)result.signed_word; break;
    case FX_U64: out->u64 = result.u64; break;
    case FX_F64: out->f64 = result.f64; break;
    case FX_POINTER: out->pointer = result.pointer; break;
    default: return BB_INVALID;
    }
    return BB_OK;
}

int bb_add_via_ffi(int32_t a, int32_t b, int32_t *out)
{
    FxPlan plan;
    FxKind kinds[] = { FX_I32, FX_I32, FX_POINTER };
    FxValue args[3] = { {.i32=a}, {.i32=b}, {.pointer=out} }, result;
    if (fx_prepare(&plan, FX_I32, kinds, 3) != BB_OK) return BB_INVALID;
    if (fx_call(&plan, FFI_FN(bb_add), kinds, args, 3, &result) != BB_OK) return BB_INVALID;
    return result.i32;
}

const BbApi *book_api_query(uint32_t version, size_t required_size)
{
    static const BbApi api = { 1, sizeof(BbApi), bb_add };
    return version == 1 && required_size <= sizeof(api) ? &api : NULL;
}
