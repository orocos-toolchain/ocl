/*
 * Lua scripting plugin
 */

#include <rtt/Service.hpp>
#include <rtt/plugin/ServicePlugin.hpp>
#include <iostream>

#include "../rtt.hpp"

extern "C" {
#include "../lua-repl.h"
void dotty (lua_State *L);
void l_message (const char *pname, const char *msg);
int dofile (lua_State *L, const char *name);
int dostring (lua_State *L, const char *s, const char *name);
}

using namespace RTT;

class LuaService : public Service
{
protected:
	lua_State *L;

public:
	LuaService(RTT::TaskContext* c)
		: RTT::Service("Lua", c)
	{
		/* initialize lua */
		L = lua_open();
		lua_gc(L, LUA_GCSTOP, 0);
		luaL_openlibs(L);
		lua_gc(L, LUA_GCRESTART, 0);

		if (L == NULL) {
			l_message("Class Lua ctr", "cannot create state: not enough memory");
			throw;
		}

		/* setup rtt bindings */
		lua_pushcfunction(L, luaopen_rtt);
		lua_call(L, 0, 0);

		this->addOperation("exec_file", &LuaService::exec_file, this);
		this->addOperation("exec_str", &LuaService::exec_str, this);
	}

	bool exec_file(const std::string &file)
	{
		if (luaL_dofile(L, file.c_str())) {
			Logger::log(Logger::Error) << lua_tostring(L, -1) << endlog();
			return false;
		}
		return true;
	}

	bool exec_str(const std::string &str)
	{
		if (luaL_dostring(L, str.c_str())) {
			Logger::log(Logger::Error) << lua_tostring(L, -1) << endlog();
			return false;
		}
		return true;
	}
};

ORO_SERVICE_NAMED_PLUGIN(LuaService , "lua")
