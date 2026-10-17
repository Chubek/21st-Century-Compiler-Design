/* Chapter 76: Lua 5.4 module, full-userdata ownership and protected callbacks. */
#include <lua.h>
#include <lauxlib.h>
#include "bookffi.h"

typedef struct { BbBuffer *buffer; } Owner;
static const char owner_type[] = "bookffi.buffer.v1";

static int32_t integer32(lua_State *L, int index)
{
    lua_Integer value;
    luaL_argcheck(L, lua_isinteger(L, index), index, "exact integer required");
    value = lua_tointeger(L, index);
    luaL_argcheck(L, value >= INT32_MIN && value <= INT32_MAX, index, "int32 range");
    return (int32_t)value;
}

static int add(lua_State *L)
{
    int32_t a = integer32(L, 1), b = integer32(L, 2), sum;
    int status = bb_add_via_ffi(a, b, &sum);
    if (status != BB_OK) return luaL_error(L, "addition failed: %d", status);
    lua_pushinteger(L, sum);
    return 1;
}

static int close_buffer(lua_State *L)
{
    Owner *owner = luaL_checkudata(L, 1, owner_type);
    bb_free(owner->buffer);
    owner->buffer = NULL;
    return 0;
}

static int create(lua_State *L)
{
    size_t size;
    const char *data;
    Owner *owner;
    int status;
    luaL_checktype(L, 1, LUA_TSTRING);
    data = lua_tolstring(L, 1, &size);
    owner = lua_newuserdatauv(L, sizeof(*owner), 0);
    owner->buffer = NULL;
    luaL_setmetatable(L, owner_type); /* Install cleanup before native acquisition. */
    status = bb_new(data, size, &owner->buffer);
    if (status != BB_OK) return luaL_error(L, "buffer creation failed: %d", status);
    return 1;
}

static int bytes_copy(lua_State *L)
{
    Owner *owner = luaL_checkudata(L, 1, owner_type);
    if (!owner->buffer) return luaL_error(L, "closed buffer");
    lua_pushlstring(L, (const char *)bb_data(owner->buffer), bb_size(owner->buffer));
    return 1;
}

typedef struct { lua_State *L; int failure; } Callback;
static int callback(void *context, int32_t index, int32_t *out)
{
    Callback *c = context;
    lua_Integer value;
    if (!lua_checkstack(c->L, 3)) { c->failure = 2; return BB_CALLBACK; }
    lua_pushvalue(c->L, 2);
    lua_pushinteger(c->L, index);
    if (lua_pcall(c->L, 1, 1, 0) != LUA_OK) {
        c->failure = 1; return BB_CALLBACK; /* Original error object stays on top. */
    }
    if (!lua_isinteger(c->L, -1)) {
        lua_pop(c->L, 1); c->failure = 2; return BB_CALLBACK;
    }
    value = lua_tointeger(c->L, -1);
    lua_pop(c->L, 1);
    if (value < INT32_MIN || value > INT32_MAX) {
        c->failure = 2; return BB_CALLBACK;
    }
    *out = (int32_t)value;
    return BB_OK;
}

static int visit(lua_State *L)
{
    int32_t n = integer32(L, 1);
    int64_t sum;
    int status;
    Callback context = {L, 0};
    luaL_checktype(L, 2, LUA_TFUNCTION);
    status = bb_visit(n, callback, &context, &sum);
    if (context.failure == 1) return lua_error(L);
    if (status != BB_OK) return luaL_error(L, "visit failed: %d", status);
    /* Build contract: standard 64-bit Lua integers. */
    lua_pushinteger(L, (lua_Integer)sum);
    return 1;
}

int luaopen_bookffi(lua_State *L)
{
    static const luaL_Reg methods[] = {
        {"bytes", bytes_copy}, {"close", close_buffer},
        {"__gc", close_buffer}, {"__close", close_buffer}, {NULL, NULL}
    };
    static const luaL_Reg functions[] = {
        {"add", add}, {"create", create}, {"visit", visit}, {NULL, NULL}
    };
    _Static_assert(sizeof(lua_Integer) >= sizeof(int64_t), "64-bit Lua integers required");
    luaL_newmetatable(L, owner_type);
    luaL_setfuncs(L, methods, 0);
    lua_pushvalue(L, -1); lua_setfield(L, -2, "__index");
    lua_pushliteral(L, "protected bookffi buffer"); lua_setfield(L, -2, "__metatable");
    lua_pop(L, 1);
    luaL_newlib(L, functions);
    return 1;
}
