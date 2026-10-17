/* C11/Lua 5.4 support for source chapters 53--60. ELF fields are byte-coded,
   never read by casting file bytes to host structs. POSIX chmod is in luald.c. */
#ifndef BOOK_TOOL_COMMON_H
#define BOOK_TOOL_COMMON_H
#include <stdint.h>
#include <inttypes.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <errno.h>
#include <limits.h>
#include <ctype.h>
#include <lua.h>
#include <lauxlib.h>
#include <lualib.h>

#define CAP (16u * 1024u * 1024u)
#define NSECT 64
#define NSYM 4096
#define NREL 8192
#define NOBJ 64
typedef struct { unsigned char *p; size_t n, cap; } Bytes;
static void *owned[8192];
static size_t nowned;
static lua_State *vm;
static const char *context = "tool";
static unsigned line_no;

static inline void fail(const char *fmt, ...)
{
    va_list ap;
    fprintf(stderr, "%s:%u: ", context, line_no);
    va_start(ap, fmt); vfprintf(stderr, fmt, ap); va_end(ap);
    fputc('\n', stderr);
    exit(2);
}
static inline void cleanup(void)
{
    if (vm) lua_close(vm);
    for (size_t i = 0; i < nowned; ++i) free(owned[i]);
}
static inline void init(void) { if (atexit(cleanup)) fail("atexit failed"); }
static inline void *alloc(size_t n)
{
    if (nowned == sizeof owned / sizeof *owned) fail("allocation count limit");
    void *p = calloc(n ? n : 1, 1);
    if (!p) fail("out of memory");
    owned[nowned++] = p;
    return p;
}
static inline void reserve(Bytes *b, size_t n)
{
    if (n > CAP) fail("16 MiB buffer limit");
    if (n <= b->cap) return;
    size_t cap = b->cap ? b->cap : 256;
    while (cap < n) cap *= 2;
    if (!b->p) b->p = alloc(cap);
    else {
        size_t i = 0;
        while (i < nowned && owned[i] != b->p) ++i;
        if (i == nowned) fail("internal allocation invariant");
        void *p = realloc(b->p, cap);
        if (!p) fail("out of memory");
        owned[i] = p; b->p = p;
    }
    b->cap = cap;
}
static inline size_t zero(Bytes *b, size_t n)
{
    if (n > CAP - b->n) fail("buffer size overflow");
    size_t at = b->n;
    reserve(b, at + n);
    if (n) memset(b->p + at, 0, n);
    b->n += n;
    return at;
}
static inline size_t append(Bytes *b, const void *p, size_t n)
{
    size_t at = zero(b, n);
    if (n) memcpy(b->p + at, p, n);
    return at;
}
static inline size_t stradd(Bytes *b, const char *s) { return append(b, s, strlen(s)+1); }
static inline void put(Bytes *b, size_t at, uint64_t v, unsigned width)
{
    if (at > b->n || width > b->n - at) fail("internal patch bounds");
    for (unsigned i = 0; i < width; ++i) b->p[at+i] = (unsigned char)(v >> (8*i));
}
static inline uint64_t get(const Bytes *b, uint64_t at, unsigned width)
{
    if (at > b->n || width > b->n - at) fail("truncated ELF field");
    uint64_t v = 0;
    for (unsigned i = 0; i < width; ++i) v |= (uint64_t)b->p[at+i] << (8*i);
    return v;
}
static inline uint64_t align_up(uint64_t v, uint64_t a)
{
    if (!a || (a & (a-1)) || a > 4096) fail("alignment must be a power of two <= 4096");
    if (v > UINT64_MAX - (a-1)) fail("alignment overflow");
    return (v + a-1) & ~(a-1);
}
static inline void pad(Bytes *b, size_t a) { zero(b, (size_t)align_up(b->n,a)-b->n); }
static inline Bytes read_file(const char *path)
{
    FILE *f = fopen(path,"rb");
    if (!f) fail("cannot read %s: %s",path,strerror(errno));
    Bytes b = {0}; unsigned char block[4096]; size_t n;
    while ((n=fread(block,1,sizeof block,f))) append(&b,block,n);
    if (ferror(f)) fail("read failure: %s",path);
    if (fclose(f)) fail("close failure: %s",path);
    return b;
}
static inline void write_file(const char *path, const Bytes *b)
{
    FILE *f=fopen(path,"wb");
    if (!f) fail("cannot write %s",path);
    int bad = fwrite(b->p,1,b->n,f) != b->n;
    if (fclose(f)) bad=1;
    if (bad) { remove(path); fail("write failure: %s",path); }
}
static inline void lua_run(const char *path, int results)
{
    vm=luaL_newstate(); if (!vm) fail("Lua allocation failure");
    luaL_openlibs(vm);
    if (luaL_loadfilex(vm,path,"t") || lua_pcall(vm,0,results,0))
        fail("Lua: %s",lua_tostring(vm,-1) ? lua_tostring(vm,-1) : "non-string error");
}
static inline void elf_header(Bytes *b, unsigned type)
{
    zero(b,64); memcpy(b->p,"\177ELF",4);
    b->p[4]=2; b->p[5]=1; b->p[6]=1;
    put(b,16,type,2); put(b,18,62,2); put(b,20,1,4); put(b,52,64,2);
}
static inline void shdr(Bytes *b, size_t at, uint32_t name, uint32_t type,
                        uint64_t flags, uint64_t off, uint64_t size,
                        uint32_t link, uint32_t info, uint64_t align, uint64_t entsize)
{
    put(b,at,name,4); put(b,at+4,type,4); put(b,at+8,flags,8);
    put(b,at+24,off,8); put(b,at+32,size,8); put(b,at+40,link,4);
    put(b,at+44,info,4); put(b,at+48,align,8); put(b,at+56,entsize,8);
}
static inline int64_t signed64(uint64_t n)
{ return n <= INT64_MAX ? (int64_t)n : -1 - (int64_t)(UINT64_MAX-n); }
#endif
