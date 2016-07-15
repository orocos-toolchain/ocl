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

#ifndef OCL_LUACOMPONENT_HPP
#define OCL_LUACOMPONENT_HPP

#include <string>
#include <rtt/os/Mutex.hpp>
#include <rtt/TaskContext.hpp>

#include "LuaStateHandle.hpp"

struct lua_State;
#ifdef LUA_RTT_TLSF
struct lua_tlsf_info;
#endif

namespace OCL
{
  class LuaComponent : public RTT::TaskContext
  {
  protected:
    std::string lua_string;
    std::string lua_file;
    lua_State *L;
    RTT::os::MutexRecursive m;

  public:
    LuaComponent(std::string name);
    ~LuaComponent();

    bool exec_file(const std::string &file);
    bool exec_str(const std::string &str);

    bool configureHook();
    bool activateHook();
    bool startHook();
    void updateHook();
    void stopHook();
    void cleanupHook();
    void errorHook();

    LuaStateHandle getLuaState();
  };

#if LUA_RTT_TLSF
  class LuaTLSFComponent : public RTT::TaskContext
  {
  protected:
    std::string lua_string;
    std::string lua_file;
    lua_State *L;
    RTT::os::MutexRecursive m;
    struct lua_tlsf_info *tlsf_inf;

  public:
    LuaTLSFComponent(std::string name);
    ~LuaTLSFComponent();

    bool tlsf_incmem(unsigned int size);

    bool exec_file(const std::string &file);
    bool exec_str(const std::string &str);

    bool configureHook();
    bool activateHook();
    bool startHook();
    void updateHook();
    void stopHook();
    void cleanupHook();
    void errorHook();

    LuaStateHandle getLuaState();
  };
#endif // LUA_RTT_TLSF
} // namespace OCL

#endif // OCL_LUACOMPONENT_HPP
