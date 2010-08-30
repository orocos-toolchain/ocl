
#define RTTLUA_VERSION	"1.0-rc1"

extern "C" {
#include <lua.h>
#include <lauxlib.h>
#include <lualib.h>
#include <string.h>
}

extern "C" int luaopen_rtt(lua_State *L);
