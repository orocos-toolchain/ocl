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

#include "LuaComponent.hpp"
#include <iostream>

#include "rtt.hpp"

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

using namespace RTT;

namespace OCL
{
  LuaComponent::LuaComponent(std::string name)
    : TaskContext(name, PreOperational)
  {
    os::MutexLock lock(m);
#if LUA_RTT_TLSF
    tlsf_inf = new lua_tlsf_info;
    if(tlsf_rtt_init_mp(tlsf_inf, TLSF_INITIAL_POOLSIZE)) {
      Logger::log(Logger::Error) << "LuaComponent '" << name << ": failed to create tlsf pool ("
               << std::hex << TLSF_INITIAL_POOLSIZE << "bytes)" << endlog();
      throw;
    }

    L = lua_newstate(tlsf_alloc, tlsf_inf);
    tlsf_inf->L = L;
    set_context_tlsf_info(tlsf_inf);
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

  LuaComponent::~LuaComponent()
  {
    os::MutexLock lock(m);
    lua_close(L);
#ifdef LUA_RTT_TLSF
    tlsf_rtt_free_mp(tlsf_inf);
    delete tlsf_inf;
#endif
  }

#ifdef LUA_RTT_TLSF
  bool LuaComponent::tlsf_incmem(unsigned int size)
  {
    return tlsf_rtt_incmem(tlsf_inf, size);
  }
#endif


  bool LuaComponent::exec_file(const std::string &file)
  {
    os::MutexLock lock(m);
    if (luaL_dofile(L, file.c_str())) {
      Logger::log(Logger::Error) << "LuaComponent '" << this->getName() << "': " << lua_tostring(L, -1) << endlog();
      return false;
    }
    return true;
  }

  bool LuaComponent::exec_str(const std::string &str)
  {
    os::MutexLock lock(m);
    if (luaL_dostring(L, str.c_str())) {
      Logger::log(Logger::Error) << "LuaComponent '" << this->getName() << "': " << lua_tostring(L, -1) << endlog();
      return false;
    }
    return true;
  }

  bool LuaComponent::configureHook()
  {
    if(!lua_string.empty())
      exec_str(lua_string);

    if(!lua_file.empty())
      exec_file(lua_file);
    return call_func(L, "configureHook", this, 0, 1);
  }

  bool LuaComponent::activateHook()
  {
    os::MutexLock lock(m);
    return call_func(L, "activateHook", this, 0, 1);
  }

  bool LuaComponent::startHook()
  {
    os::MutexLock lock(m);
    return call_func(L, "startHook", this, 0, 1);
  }

  void LuaComponent::updateHook()
  {
    os::MutexLock lock(m);
    call_func(L, "updateHook", this, 0, 0);
  }

  void LuaComponent::stopHook()
  {
    os::MutexLock lock(m);
    call_func(L, "stopHook", this, 0, 0);
  }

  void LuaComponent::cleanupHook()
  {
    os::MutexLock lock(m);
    call_func(L, "cleanupHook", this, 0, 0);
  }

  void LuaComponent::errorHook()
  {
    os::MutexLock lock(m);
    call_func(L, "errorHook", this, 0, 0);
  }

  LuaStateHandle LuaComponent::getLuaState()
  {
    return LuaStateHandle(L, m);
  }

} // namespace OCL

#include "ocl/Component.hpp"
ORO_CREATE_COMPONENT( OCL::LuaComponent )
