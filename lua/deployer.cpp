/*
 * Lua RTT bindings
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
