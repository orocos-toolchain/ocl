/*
 * Lua-RTT bindings: Lua module for creating a deployer.
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
#include <lua.h>
#include <lauxlib.h>
#include <lualib.h>

#include <string.h>
}

#include <rtt/TaskContext.hpp>
#include <ocl/OCL.hpp>
#include <deployment/DeploymentComponent.hpp>

using namespace std;
using namespace OCL;
using namespace Orocos;

/* template for generating GC function */
template<typename T>
int GCMethod(lua_State* L)
{
	reinterpret_cast<T*>(lua_touserdata(L, 1))->~T();
	return 0;
}

/*
 * Deployer
 */
static int deployer_new(lua_State *L)
{
	const char *s;
	std::string str;

	s = luaL_checkstring(L, 1);
	str = s;
	DeploymentComponent *d = new DeploymentComponent(str);
	TaskContext** tc = (TaskContext**) lua_newuserdata(L, sizeof(TaskContext*));

	*tc = (TaskContext*) d;
	luaL_getmetatable(L, "TaskContext");
	lua_setmetatable(L, -2);
	return 1;
}

/* only explicit destrutction of deployers */
static const struct luaL_Reg DeploymentComponent_f [] = {
	{"new", deployer_new },
	{NULL, NULL}
};

static const struct luaL_Reg DeploymentComponent_m [] = {
	/* {"__gc", deployer_gc }, */
	{NULL, NULL}
};

extern "C" {
int luaopen_deployer(lua_State *L)
{
	/* register MyObj
	 * 1. line creates metatable MyObj and registers name in registry
	 * 2. line duplicates metatable
	 * 3. line sets metatable[__index]=metatable
	 * 4. line register methods in metatable
	 * 5. line registers free functions in global mystuff.MyObj table
	 */
	luaL_newmetatable(L, "TaskContext");
	lua_pushvalue(L, -1);			/* duplicates metatable */
	lua_setfield(L, -2, "__index");
	luaL_register(L, NULL, DeploymentComponent_m);
	luaL_register(L, "deployer", DeploymentComponent_f);

	return 1;
}
} /* extern "C" */
