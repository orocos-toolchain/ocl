
#include <lua.h>
#include <lauxlib.h>
#include <lualib.h>

#define TLSF_INITIAL_POOLSIZE	1*1024*1024
#undef	TLSF_DEBUG

/* this is used as the opque Lua userdata to the alloc func */
struct lua_tlsf_info {
	void *pool;
	void *pool2;
	unsigned int total_mem;
	unsigned int mask;
	lua_State *L;
};

int tlsf_rtt_init_mp(struct lua_tlsf_info *tlsf_inf, size_t sz);
void tlsf_rtt_free_mp(struct lua_tlsf_info *tlsf_inf);
void* tlsf_alloc (void *ud, void *ptr, size_t osize, size_t nsize);
int tlsf_rtt_incmem(struct lua_tlsf_info *tlsf_inf, size_t sz);
void set_context_tlsf_info(struct lua_tlsf_info*);
void register_tlsf_api(lua_State *L);

