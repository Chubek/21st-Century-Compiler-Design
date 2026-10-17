/* Chapters 69--76: C11 native boundary with explicit lengths and ownership.
   Build dependencies and supported platform are documented in README.md. */
#ifndef BOOKFFI_H
#define BOOKFFI_H
#include <stddef.h>
#include <stdint.h>
#include <ffi.h>

enum { BB_OK, BB_INVALID, BB_RANGE, BB_NOMEM, BB_CALLBACK };
enum { BB_LIMIT = 1048576, BB_VISIT_LIMIT = 10000, FX_MAX_ARGS = 8 };
typedef struct BbBuffer BbBuffer;
int bb_add(int32_t a, int32_t b, int32_t *out);
int bb_new(const void *data, size_t size, BbBuffer **out);
void bb_free(BbBuffer *buffer);
const unsigned char *bb_data(const BbBuffer *buffer);
size_t bb_size(const BbBuffer *buffer);
int bb_read(const BbBuffer *buffer, size_t offset, size_t count, void *out);
typedef int (*BbCallback)(void *context, int32_t index, int32_t *out);
int bb_visit(int32_t count, BbCallback callback, void *context, int64_t *out);

typedef enum { FX_I32, FX_U64, FX_F64, FX_POINTER } FxKind;
typedef union { int32_t i32; uint64_t u64; double f64; void *pointer; } FxValue;
typedef struct {
    ffi_cif cif;
    ffi_type *types[FX_MAX_ARGS];
    FxKind kinds[FX_MAX_ARGS], result;
    size_t count;
    int ready;
} FxPlan;
/* A prepared plan is address-stable, caller-owned, and not copied or moved. */
int fx_prepare(FxPlan *plan, FxKind result, const FxKind *kinds, size_t count);
int fx_call(FxPlan *plan, void (*function)(void), const FxKind *kinds,
            FxValue *values, size_t count, FxValue *out);
int bb_add_via_ffi(int32_t a, int32_t b, int32_t *out);

/* Native same-platform ABI, not a serialized/wire structure. */
typedef struct {
    uint32_t version;
    size_t size;
    int (*add)(int32_t, int32_t, int32_t *);
} BbApi;
const BbApi *book_api_query(uint32_t version, size_t required_size);
#endif
