/**
 * @file Lua.cpp
 * Simple sample LuaComponent
 */

extern "C" {
#include "lua-repl.h"
void dotty (lua_State *L);
void l_message (const char *pname, const char *msg);
int dofile (lua_State *L, const char *name);
int dostring (lua_State *L, const char *s, const char *name);
}

#include "rtt.hpp"

#include <string>
#include <rtt/os/main.h>
#include <rtt/TaskContext.hpp>

// #include <taskbrowser/TaskBrowser.hpp>
// #include <rtt/Logger.hpp>
// #include <rtt/Property.hpp>
// #include <rtt/Attribute.hpp>
// #include <rtt/Method.hpp>
// #include <rtt/Port.hpp>

#include <ocl/OCL.hpp>
#include <deployment/DeploymentComponent.hpp>

// #include "class-lua/RTTLua.hpp"

using namespace std;
using namespace RTT;
using namespace Orocos;

namespace OCL
{
	class LuaComponent : public TaskContext
	{
	protected:
		lua_State *L;

	public:
		LuaComponent(std::string name)
			: TaskContext(name, PreOperational)
		{
			Logger::In in("constructor()");
			
			L = lua_open();
			lua_gc(L, LUA_GCSTOP, 0);
			luaL_openlibs(L);
			lua_gc(L, LUA_GCRESTART, 0);

			if (L == NULL) {
				l_message("Class Lua ctr", "cannot create state: not enough memory");
				throw;
			}

			/* setup rtt bindings */
			luaopen_rtt(L);

			/* set global TC */
			TaskContext** tc = (TaskContext**) lua_newuserdata(L, sizeof(TaskContext*));
			*tc = (TaskContext*) this;
			luaL_getmetatable(L, "TaskContext");
			lua_setmetatable(L, -2);
			lua_setglobal(L, "TC");

			this->addOperation("exec_file", &LuaComponent::exec_file, this, OwnThread)
				.doc("load (and run) the given lua script")
				.arg("filename", "filename of the lua script");

			this->addOperation("exec_str", &LuaComponent::exec_str, this, OwnThread)
				.doc("evaluate the given string in the lua environment")
				.arg("lua-string", "string of lua code to evaluate");
		}

		~LuaComponent()
		{
			lua_close(L);
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

		void lua_repl()
		{
			cout << "Orocos RTTLua " << RTTLUA_VERSION << " (" << OROCOS_TARGET_NAME << ")"  << endl;
			dotty(L);
		}

		bool configureHook()
		{
			return exec_str("configureHook()");
		}

		bool startHook()
		{
			return exec_str("startHook()");
		}

		void updateHook()
		{
			exec_str("updateHook()");
		}

		void stopHook()
		{
			exec_str("stopHook()");
		}

		void cleanupHook()
		{
			exec_str("cleanupHook()");
		}
	};
}


#ifndef OCL_COMPONENT_ONLY

int ORO_main(int argc, char** argv)
{
	Logger::In in("main()");

	if ( log().getLogLevel() < Logger::Info ) {
		log().setLogLevel( Logger::Info );
	}
	
	LuaComponent lua("lua");
	DeploymentComponent dc("deployer");
	lua.connectPeers(&dc);

	if(argc>1) {
		Logger::log(Logger::Info) << "executing script: " << argv[1] << endlog();
		lua.exec_file(argv[1]);
	}
	lua.lua_repl();
	return 0;
}

#else

#include "ocl/Component.hpp"
ORO_CREATE_COMPONENT( OCL::LuaComponent )
#endif
