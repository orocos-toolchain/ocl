/*
 * Lua scripting plugin
 */

#include <rtt/Service.hpp>
#include <rtt/plugin/ServicePlugin.hpp>
#include <iostream>

#include "../rtt.hpp"

#ifdef LUA_RTT_TLSF
extern "C" {
#include "../tlsf_rtt.h"
}
#endif

#ifdef LUA_RTT_TLSF
#define LuaService LuaTLSFService
#else
#define LuaService LuaService
#endif

using namespace RTT;

class LuaService : public Service
{
protected:
	lua_State *L;
	os::Mutex m;
#if LUA_RTT_TLSF
	struct lua_tlsf_info tlsf_inf;
#endif

public:
	LuaService(RTT::TaskContext* tc)
#if LUA_RTT_TLSF
		: RTT::Service("LuaTLSF", tc)
#else
		: RTT::Service("Lua", tc)
#endif
	{
		/* initialize lua */
		os::MutexLock lock(m);

#if LUA_RTT_TLSF
		if(tlsf_rtt_init_mp(&tlsf_inf, TLSF_INITIAL_POOLSIZE)) {
			Logger::log(Logger::Error) << "LuaService (TLSF)'"
						   << this->getOwner()->getName() << ": failed to create tlsf pool ("
						   << std::hex << TLSF_INITIAL_POOLSIZE << "bytes)" << endlog();
			throw;
		}

		L = lua_newstate(tlsf_alloc, &tlsf_inf);
		tlsf_inf.L = L;
		set_context_tlsf_info(&tlsf_inf);
		register_tlsf_api(L);
#else
		L = luaL_newstate();
#endif

		if (L == NULL) {
			Logger::log(Logger::Error) << "LuaService ctr '" << this->getOwner()->getName() << "': "
						   << "cannot create state: not enough memory" << endlog();
			throw;
		}


		lua_gc(L, LUA_GCSTOP, 0);
		luaL_openlibs(L);
		lua_gc(L, LUA_GCRESTART, 0);

		/* setup rtt bindings */
		lua_pushcfunction(L, luaopen_rtt);
		lua_call(L, 0, 0);

		set_context_tc(tc, L);

		this->addOperation("exec_file", &LuaService::exec_file, this)
			.doc("load (and run) the given lua script")
			.arg("filename", "filename of the lua script");

		this->addOperation("exec_str", &LuaService::exec_str, this)
			.doc("evaluate the given string in the lua environment")
			.arg("lua-string", "string of lua code to evaluate");

#ifdef LUA_RTT_TLSF
		this->addOperation("tlsf_incmem", &LuaService::tlsf_incmem, this, OwnThread)
			.doc("increase the TLSF memory pool")
			.arg("size", "size in bytes to add to pool");
#endif
	}

	// Destructor
	~LuaService()
	{
		os::MutexLock lock(m);
		lua_close(L);
#ifdef LUA_RTT_TLSF
		tlsf_rtt_free_mp(&tlsf_inf);
#endif
	}


#ifdef LUA_RTT_TLSF
	bool tlsf_incmem(unsigned int size)
	{
		return tlsf_rtt_incmem(&tlsf_inf, size);
	}
#endif
	bool exec_file(const std::string &file)
	{
		os::MutexLock lock(m);
		if (luaL_dofile(L, file.c_str())) {
			Logger::log(Logger::Error) << "LuaService '" << this->getOwner()->getName()
						   << "': " << lua_tostring(L, -1) << endlog();
			return false;
		}
		return true;
	}

	bool exec_str(const std::string &str)
	{
		os::MutexLock lock(m);
		if (luaL_dostring(L, str.c_str())) {
			Logger::log(Logger::Error) << "LuaService '" << this->getOwner()->getName()
						   << "': " << lua_tostring(L, -1) << endlog();
			return false;
		}
		return true;
	}
};

#ifdef LUA_RTT_TLSF
 ORO_SERVICE_NAMED_PLUGIN(LuaService , "LuaTLSF")
#else
 ORO_SERVICE_NAMED_PLUGIN(LuaService , "Lua")
#endif
