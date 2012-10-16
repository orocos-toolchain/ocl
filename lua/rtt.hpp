/*
 * Lua-RTT bindings
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

#include <rtt/TaskContext.hpp>
#include <rtt/Port.hpp>
#include <rtt/types/Types.hpp>
#include <rtt/base/DataSourceBase.hpp>
#include <rtt/types/Operators.hpp>
#include <rtt/Logger.hpp>
#include <rtt/plugin/PluginLoader.hpp>
#include <rtt/os/TimeService.hpp>
#include <rtt/os/fosi.h>
#include <rtt/internal/GlobalService.hpp>
#include <rtt/types/GlobalsRepository.hpp>

extern "C" {
#include <lua.h>
#include <lauxlib.h>
#include <lualib.h>
#include <string.h>

#include <stdint.h>

int luaopen_rtt(lua_State *L);
int set_context_tc(RTT::TaskContext*, lua_State*);

/* call a function/0 named by string, the last two boolean arguments
 * are wether to fail if no such function exists and wether to fail if
 * no boolean result is returned.
 */
bool call_func(lua_State*, const char*, RTT::TaskContext*, int, int);
}


