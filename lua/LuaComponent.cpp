/*
 * Lua-RTT bindings. LuaComponent.
 *
 * (C) Copyright 2010 Markus Klotzbuecher
 * markus.klotzbuecher@mech.kuleuven.be
 * Department of Mechanical Engineering,
 * Katholieke Universiteit Leuven, Belgium.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public
 * License as published by the Free Software Foundation;
 * version 2 of the License.
 *
 * As a special exception, you may use this file as part of a free
 * software library without restriction.  Specifically, if other files
 * instantiate templates or use macros or inline functions from this
 * file, or you compile this file and link it with other files to
 * produce an executable, this file does not by itself cause the
 * resulting executable to be covered by the GNU General Public
 * License.  This exception does not however invalidate any other
 * reasons why the executable file might be covered by the GNU General
 * Public License.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place,
 * Suite 330, Boston, MA  02111-1307  USA
 */

extern "C" {
#include "lua-repl.h"
void dotty (lua_State *L);
void l_message (const char *pname, const char *msg);
int dofile (lua_State *L, const char *name);
int dostring (lua_State *L, const char *s, const char *name);

#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <wordexp.h>
}

#include "rtt.hpp"

#include <string>
#include <rtt/os/main.h>
#include <rtt/TaskContext.hpp>
#include <ocl/OCL.hpp>
#include <deployment/DeploymentComponent.hpp>

#define INIT_FILE	"~/.rttlua"

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

			set_context_tc(this, L);

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
	struct stat stb;
	wordexp_t init_exp;

  	// log().setLogLevel( Logger::Warning );

	LuaComponent lua("lua");
	DeploymentComponent dc("deployer");
	lua.connectPeers(&dc);

	/* run init file */
	wordexp(INIT_FILE, &init_exp, 0);
	if(stat(init_exp.we_wordv[0], &stb) != -1) {
		if((stb.st_mode & S_IFMT) != S_IFREG)
			cout << "rttlua: warning: init file " << init_exp.we_wordv[0] << " is not a regular file" << endl;
		else
			lua.exec_file(init_exp.we_wordv[0]);
	}
	wordfree(&init_exp);

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
