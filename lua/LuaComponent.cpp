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

#ifndef OCL_COMPONENT_ONLY

#include <rtt/rtt-config.h>
#ifdef OS_RT_MALLOC
// need access to all TLSF functions embedded in RTT
#define ORO_MEMORY_POOL
#include <rtt/os/tlsf/tlsf.h>
#endif
#include <rtt/os/main.h>
#include <rtt/RTT.hpp>
#include <rtt/Logger.hpp>
#ifdef  ORO_BUILD_LOGGING
#   ifndef OS_RT_MALLOC
#   warning "Logging needs rtalloc!"
#   endif
#include <log4cpp/HierarchyMaintainer.hh>
#include "logging/Category.hpp"
#endif

extern "C" {
#include "lua-repl.h"
void dotty (lua_State *L);
void l_message (const char *pname, const char *msg);
int dofile (lua_State *L, const char *name);
int dostring (lua_State *L, const char *s, const char *name);
int main_args(lua_State *L, int argc, char **argv);

#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <wordexp.h>
}
#endif

#include "rtt.hpp"

#include <string>
#include <rtt/os/main.h>
#include <rtt/os/Mutex.hpp>
#include <rtt/os/MutexLock.hpp>
#include <rtt/TaskContext.hpp>
#include <ocl/OCL.hpp>
#if defined(LUA_RTT_CORBA)
#include <rtt/transports/corba/TaskContextServer.hpp>
#include <deployment/CorbaDeploymentComponent.hpp>
#else
#include <deployment/DeploymentComponent.hpp>
#endif

#ifdef LUA_RTT_TLSF
extern "C" {
#include "tlsf_rtt.h"
}
#endif

#ifdef LUA_RTT_TLSF
#define LuaComponent LuaTLSFComponent
#else
#define LuaComponent LuaComponent
#endif

#define INIT_FILE	"~/.rttlua"

using namespace std;
using namespace RTT;
using namespace Orocos;
#if defined(LUA_RTT_CORBA)
using namespace RTT::corba;
#endif

namespace OCL
{
	class LuaComponent : public TaskContext
	{
	protected:
		std::string lua_string;
		std::string lua_file;
		lua_State *L;
		os::MutexRecursive m;
#if LUA_RTT_TLSF
		struct lua_tlsf_info tlsf_inf;
#endif

	public:
		LuaComponent(std::string name)
			: TaskContext(name, PreOperational)
		{
			os::MutexLock lock(m);
#if LUA_RTT_TLSF
			if(tlsf_rtt_init_mp(&tlsf_inf, TLSF_INITIAL_POOLSIZE)) {
				Logger::log(Logger::Error) << "LuaComponent '" << name << ": failed to create tlsf pool ("
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
				Logger::log(Logger::Error) << "LuaComponent '" << name
							   << "': failed to allocate memory for Lua state" << endlog();
				throw;
			}

			lua_gc(L, LUA_GCSTOP, 0);
			luaL_openlibs(L);
			lua_gc(L, LUA_GCRESTART, 0);

			/* setup rtt bindings */
			lua_pushcfunction(L, luaopen_rtt);
			lua_call(L, 0, 0);

			set_context_tc(this, L);

			this->addProperty("lua_string", lua_string).doc("string of lua code to be executed during configureHook");
			this->addProperty("lua_file", lua_file).doc("file with lua program to be executed during configuration");

			this->addOperation("exec_file", &LuaComponent::exec_file, this, OwnThread)
				.doc("load (and run) the given lua script")
				.arg("filename", "filename of the lua script");

			this->addOperation("exec_str", &LuaComponent::exec_str, this, OwnThread)
				.doc("evaluate the given string in the lua environment")
				.arg("lua-string", "string of lua code to evaluate");

#ifdef LUA_RTT_TLSF
			this->addOperation("tlsf_incmem", &LuaComponent::tlsf_incmem, this, OwnThread)
				.doc("increase the TLSF memory pool")
				.arg("size", "size in bytes to add to pool");
#endif
		}

		~LuaComponent()
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
				Logger::log(Logger::Error) << "LuaComponent '" << this->getName() << "': " << lua_tostring(L, -1) << endlog();
				return false;
			}
			return true;
		}

		bool exec_str(const std::string &str)
		{
			os::MutexLock lock(m);
			if (luaL_dostring(L, str.c_str())) {
				Logger::log(Logger::Error) << "LuaComponent '" << this->getName() << "': " << lua_tostring(L, -1) << endlog();
				return false;
			}
			return true;
		}

#ifndef OCL_COMPONENT_ONLY
		void lua_repl()
		{
			os::MutexLock lock(m);
			dotty(L);
		}

		int lua_repl(int argc, char **argv)
		{
			os::MutexLock lock(m);
			return main_args(L, argc, argv);
		}
#endif
		bool configureHook()
		{
			if(!lua_string.empty())
				exec_str(lua_string);

			if(!lua_file.empty())
				exec_file(lua_file);
			return call_func(L, "configureHook", this, 0, 1);
		}

		bool activateHook()
		{
			os::MutexLock lock(m);
			return call_func(L, "activateHook", this, 0, 1);
		}

		bool startHook()
		{
			os::MutexLock lock(m);
			return call_func(L, "startHook", this, 0, 1);
		}

		void updateHook()
		{
			os::MutexLock lock(m);
			call_func(L, "updateHook", this, 0, 0);
		}

		void stopHook()
		{
			os::MutexLock lock(m);
			call_func(L, "stopHook", this, 0, 0);
		}

		void cleanupHook()
		{
			os::MutexLock lock(m);
			call_func(L, "cleanupHook", this, 0, 0);
		}

		void errorHook()
		{
			os::MutexLock lock(m);
			call_func(L, "errorHook", this, 0, 0);
		}
	};
}


#ifndef OCL_COMPONENT_ONLY

int ORO_main(int argc, char** argv)
{
	struct stat stb;
	wordexp_t init_exp;

#ifdef  ORO_BUILD_RTALLOC
    size_t                  memSize     = ORO_DEFAULT_RTALLOC_SIZE;
    void*                   rtMem       = 0;
    size_t                  freeMem     = 0;
    if (0 < memSize)
    {
        // don't calloc() as is first thing TLSF does.
        rtMem = malloc(memSize);
        assert(0 != rtMem);
        freeMem = init_memory_pool(memSize, rtMem);
        if ((size_t)-1 == freeMem)
        {
            cerr << "Invalid memory pool size of " << memSize 
                          << " bytes (TLSF has a several kilobyte overhead)." << endl;
            free(rtMem);
            return -1;
        }
        cout << "Real-time memory: " << freeMem << " bytes free of "
                  << memSize << " allocated." << endl;
    }
#endif  // ORO_BUILD_RTALLOC

#ifdef  ORO_BUILD_LOGGING
    log4cpp::HierarchyMaintainer::set_category_factory(
        OCL::logging::Category::createOCLCategory);
#endif

	LuaComponent lua("lua");
	DeploymentComponent * dc = 0;

#if defined(LUA_RTT_CORBA)
	int  orb_argc = argc;
	char** orb_argv = 0;
	char* orb_sep = 0;
        
	/* find the "--" separator */
	while(orb_argc) {
	  if(0 == strcmp("--", argv[argc - orb_argc])) {
	    orb_sep = argv[argc - orb_argc];
	    argv[argc - orb_argc] = argv[0];
	    orb_argv = &argv[argc - orb_argc];
	    argc -= orb_argc;
	    break;
	  }
	  orb_argc--;
	}
        
	/* if the "--" separator is found perhaps we have orb arguments */
	if(orb_argc) {
	  try {
	    TaskContextServer::InitOrb(orb_argc, orb_argv);
   
	    dc = new CorbaDeploymentComponent("Deployer");
   
	    TaskContextServer::Create( dc, true, true );
   
	    // The orb thread accepts incomming CORBA calls.
	    TaskContextServer::ThreadOrb();
	  } 
	  catch( CORBA::Exception &e ) {
	    log(Error) << argv[0] <<" ORO_main : CORBA exception raised!" << Logger::nl;
	    log() << CORBA_EXCEPTION_INFO(e) << endlog();
	    if(dc)
	    {
	      delete dc;
	      dc = 0;
	    }
	  } catch (...) {
	    log(Error) << "Uncaught exception." << endlog();
	    if(dc)
	    {
	      delete dc;
	      dc = 0;
	    }
	  }
 
	  argv[argc] = dc?NULL:orb_sep;
	}

	/* fallback to the default deployer if corba have failed to provide one */
	if(!dc)
#endif
	dc = new DeploymentComponent("Deployer");

	lua.connectPeers(dc);

	/* run init file */
	wordexp(INIT_FILE, &init_exp, 0);
	if(stat(init_exp.we_wordv[0], &stb) != -1) {
		if((stb.st_mode & S_IFMT) != S_IFREG)
			cout << "rttlua: warning: init file " << init_exp.we_wordv[0] << " is not a regular file" << endl;
		else
			lua.exec_file(init_exp.we_wordv[0]);
	}
	wordfree(&init_exp);

	lua.lua_repl(argc, argv);

#if defined(LUA_RTT_CORBA)
	if(orb_argc) {
	  TaskContextServer::ShutdownOrb();
	  TaskContextServer::DestroyOrb();
	}
#endif

	delete dc;

#ifdef  ORO_BUILD_LOGGING
    log4cpp::HierarchyMaintainer::getDefaultMaintainer().shutdown();
    log4cpp::HierarchyMaintainer::getDefaultMaintainer().deleteAllCategories();
#endif

#ifdef  ORO_BUILD_RTALLOC
    if (0 != rtMem)
    {
        std::cout << "TLSF bytes allocated=" << memSize
                  << " overhead=" << (memSize - freeMem)
                  << " max-used=" << get_max_size(rtMem)
                  << " currently-used=" << get_used_size(rtMem)
                  << " still-allocated=" << (get_used_size(rtMem) - (memSize - freeMem))
                  << "\n";

        destroy_memory_pool(rtMem);
        free(rtMem);
    }
#endif
	return 0;
}

#else

#include "ocl/Component.hpp"

ORO_CREATE_COMPONENT( OCL::LuaComponent )
#endif
