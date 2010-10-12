/*
 * Lua scripting plugin
 */

#include <rtt/Service.hpp>
#include <rtt/plugin/ServicePlugin.hpp>
#include <iostream>

#include "../rtt.hpp"

using namespace RTT;

class LuaService : public Service
{
protected:
	lua_State *L;

public:
	LuaService(RTT::TaskContext* tc)
		: RTT::Service("Lua", tc)
	{
		/* initialize lua */
		L = lua_open();
		lua_gc(L, LUA_GCSTOP, 0);
		luaL_openlibs(L);
		lua_gc(L, LUA_GCRESTART, 0);

		if (L == NULL) {
		  Logger::In in("LuaService ctr");
		  log(Error)<<"cannot create state: not enough memory"<<endlog();
		  throw;
		}

		/* setup rtt bindings */
		lua_pushcfunction(L, luaopen_rtt);
		lua_call(L, 0, 0);

		set_context_tc(tc, L);

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

ORO_SERVICE_NAMED_PLUGIN(LuaService , "Lua")
