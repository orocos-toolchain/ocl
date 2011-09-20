
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>

#include "tlsf.h"
#include "tlsf_rtt.h"

#define RTL_TLSF_DEBUG		1

#define DEBUG_TLSF_ALLOC	(1<<0)
#define DEBUG_TLSF_FREE		(1<<1)
#define DEBUG_TLSF_TRACE	(1<<20)

#ifdef RTL_TLSF_DEBUG
# define _DBG(x, mask, fmt, args...) do{ if (mask & x) printf("%s: " fmt "\n", __FUNCTION__, ##args); } while(0);
#else
# define _DBG(x, mask, fmt, args...) do { } while(0);
#endif

#define TLSF_POOL_MIN_SIZE	1*1014*1024

/* create a memory pool of sz and initialize it for use with TLSF */
int tlsf_rtt_init_mp(struct lua_tlsf_info *tlsf_inf, size_t sz)
{
	tlsf_inf->L = NULL;
	tlsf_inf->mask = 0;
	tlsf_inf->pool2 = NULL;
	tlsf_inf->total_mem = 0;

	if(sz < TLSF_POOL_MIN_SIZE) {
		fprintf(stderr, "error: requested tlsf pool size (0x%lx) too small\n", (unsigned long) sz);
		goto fail;
	}

	tlsf_inf->pool = malloc(sz);

	if(!tlsf_inf->pool) {
		fprintf(stderr, "error failed to allocate: 0x%lx bytes\n", (unsigned long) sz);
		goto fail;
	}

	tlsf_inf->total_mem = rtl_init_memory_pool(sz, tlsf_inf->pool);
	return 0;
 fail:
	return -1;
}

/* cleanup mempool */
void tlsf_rtt_free_mp(struct lua_tlsf_info *tlsf_inf)
{
	 rtl_destroy_memory_pool(tlsf_inf->pool);
	 free(tlsf_inf->pool);

	 if(tlsf_inf->pool2)
		 free(tlsf_inf->pool2);
}

/* this hook will print a backtrace and reset itself */
static void tlsf_trace_hook(lua_State *L, lua_Debug *ar)
{
	(void)ar;
	lua_sethook(L, tlsf_trace_hook, 0, 0);
	luaL_error(L, "memory allocation in TLSF trace mode");
}

/* tlsf based Lua allocator */
void* tlsf_alloc (void *ud, void *ptr, size_t osize, size_t nsize)
{
	(void)osize;
	struct lua_tlsf_info *tlsf_inf = (struct lua_tlsf_info*) ud;

	if (nsize == 0) {
		_DBG(DEBUG_TLSF_FREE, tlsf_inf->mask, "freeing 0x%lx, osize=%lu, nsize=%lu",
		     (unsigned long) ptr, (unsigned long) osize, (unsigned long) nsize);
		rtl_free_ex(ptr, tlsf_inf->pool);
		return NULL;
	} else {
		if(DEBUG_TLSF_TRACE & tlsf_inf->mask) {
			lua_sethook(tlsf_inf->L, tlsf_trace_hook, LUA_MASKCALL | LUA_MASKRET | LUA_MASKLINE | LUA_MASKCOUNT, 1);
		}
		_DBG(DEBUG_TLSF_ALLOC, tlsf_inf->mask, "allocating 0x%lx, osize=%lu, nsize=%lu",
		     (unsigned long) ptr, (unsigned long) osize, (unsigned long) nsize);
		return rtl_realloc_ex(ptr, nsize, tlsf_inf->pool);
	}
}

int tlsf_rtt_incmem(struct lua_tlsf_info *tlsf_inf, size_t sz)
{
	if(tlsf_inf->pool2 != NULL)
		luaL_error(tlsf_inf->L, "tlsf_rtt_incmem: region already increased, (increasing cur. only possible once)");

	if((tlsf_inf->pool2 = malloc(sz)) == NULL)
		luaL_error(tlsf_inf->L, "tlsf_rtt_incmem: failed to increase memory by %d bytes. Out of mem.");

	tlsf_inf->total_mem += rtl_add_new_area(tlsf_inf->pool2, sz, tlsf_inf->pool);

	return 0;
}

/* store and retrieve the tlsf_info in the registry
 * this is required for the enabling and disabling
 * trace functions
 */
void set_context_tlsf_info(struct lua_tlsf_info* tlsf_inf)
{
	lua_pushstring(tlsf_inf->L, "tlsf_info");
	lua_pushlightuserdata(tlsf_inf->L, tlsf_inf);
	lua_rawset(tlsf_inf->L, LUA_REGISTRYINDEX);
}

struct lua_tlsf_info* get_context_tlsf_info(lua_State *L)
{
	lua_pushstring(L, "tlsf_info");
	lua_rawget(L, LUA_REGISTRYINDEX);
	return (struct lua_tlsf_info*) lua_touserdata(L, -1);
}

static int tlsf_trace(lua_State *L)
{
	int argc, enable, ret;
	struct lua_tlsf_info *tlsf_inf = get_context_tlsf_info(L);
	ret = 0;
	argc = lua_gettop(L);

	if(argc == 0) {
		lua_pushboolean(L, tlsf_inf->mask & DEBUG_TLSF_TRACE);
		ret = 1;
		goto out;
	}

	enable = lua_toboolean(L, 1);

	if(enable) {
		tlsf_inf->mask |= DEBUG_TLSF_TRACE;
	} else {
		lua_sethook(L, tlsf_trace_hook, 0, 1);
		tlsf_inf->mask &= ~DEBUG_TLSF_TRACE;
	}

out:
	return ret;
}

static int tlsf_warn(lua_State *L)
{
	int argc, enable, ret;
	struct lua_tlsf_info *tlsf_inf = get_context_tlsf_info(L);
	ret = 0;
	argc = lua_gettop(L);

	if(argc == 0) {
		lua_pushboolean(L, tlsf_inf->mask & (DEBUG_TLSF_ALLOC | DEBUG_TLSF_FREE));
		ret = 1;
		goto out;
	}

	enable = lua_toboolean(L, 1);

	if(enable)
		tlsf_inf->mask |= DEBUG_TLSF_ALLOC | DEBUG_TLSF_FREE;
	else
		tlsf_inf->mask &= ~(DEBUG_TLSF_ALLOC | DEBUG_TLSF_FREE);
 out:
	return ret;
}

static int tlsf_stats(lua_State *L)
{
	struct lua_tlsf_info *tlsf_inf = get_context_tlsf_info(L);

	lua_pushinteger(L, rtl_get_used_size(tlsf_inf->pool));
	lua_pushinteger(L, rtl_get_max_size(tlsf_inf->pool));
	lua_pushinteger(L, tlsf_inf->total_mem);
	return 3;
}

static const struct luaL_Reg tlsf_f [] = {
	{"stats", tlsf_stats },
	{"warn", tlsf_warn },
	{"trace", tlsf_trace },
	{NULL, NULL}
};

void register_tlsf_api(lua_State *L)
{
	luaL_register(L, "tlsf", tlsf_f);
}
