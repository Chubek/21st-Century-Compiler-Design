/* Chapter 76: embedding uses the same module entry, under a protected call. */
#include <lua.h>
#include <lauxlib.h>
#include <lualib.h>
#include <stdio.h>
int luaopen_bookffi(lua_State *L);
static int run(lua_State *L)
{
    const char *path = lua_touserdata(L, 1);
    luaL_openlibs(L);
    luaL_requiref(L, "bookffi", luaopen_bookffi, 0);
    lua_pop(L, 1);
    if (luaL_loadfile(L, path) != LUA_OK) return lua_error(L);
    lua_call(L, 0, 0); /* The host's outer pcall owns this error boundary. */
    return 0;
}
int main(int argc, char **argv)
{
    lua_State *L;
    int status;
    if (argc != 2) return 2;
    L = luaL_newstate();
    if (!L) return 2;
    lua_pushcfunction(L, run);
    lua_pushlightuserdata(L, argv[1]);
    status = lua_pcall(L, 1, 0, 0);
    if (status != LUA_OK) {
        const char *message = lua_type(L, -1) == LUA_TSTRING ? lua_tostring(L, -1) : NULL;
        fprintf(stderr, "%s\n", message ? message : "non-string Lua error");
    }
    lua_close(L);
    return status == LUA_OK ? 0 : 1;
}
