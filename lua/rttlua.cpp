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

#include "LuaComponent.hpp"
#include <rtt/os/main.h>
//#include <rtt/os/Mutex.hpp>
//#include <rtt/os/MutexLock.hpp>
//#include <rtt/TaskContext.hpp>
//#include <ocl/OCL.hpp>
#if defined(LUA_RTT_CORBA)
#include <rtt/transports/corba/TaskContextServer.hpp>
#include <deployment/CorbaDeploymentComponent.hpp>
#else
#include <deployment/DeploymentComponent.hpp>
#endif

#ifdef LUA_RTT_TLSF
#define LuaComponent LuaTLSFComponent
#else
#define LuaComponent LuaComponent
#endif

#define INIT_FILE	"~/.rttlua"

using namespace std;
using namespace RTT;
using namespace OCL;
#if defined(LUA_RTT_CORBA)
using namespace RTT::corba;
#endif

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

  main_args(lua.getLuaState().get(), argc, argv);

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
