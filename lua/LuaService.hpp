#ifndef OCL_LUASERVICE_HPP
#define OCL_LUASERVICE_HPP

/*
 * Lua scripting plugin
 */

#include <rtt/Service.hpp>
#include <rtt/os/Mutex.hpp>

#include "LuaStateHandle.hpp"

struct lua_State;
#ifdef LUA_RTT_TLSF
struct lua_tlsf_info;
#endif

namespace OCL
{
  class LuaService : public RTT::Service
  {
  protected:
    lua_State *L;
    RTT::os::Mutex m;

  public:
    LuaService(RTT::TaskContext* tc);
    ~LuaService();

    bool exec_file(const std::string &file);
    bool exec_str(const std::string &str);

    LuaStateHandle getLuaState();
  };

#ifdef LUA_RTT_TLSF
  class LuaTLSFService : public RTT::Service
  {
  protected:
    lua_State *L;
    RTT::os::Mutex m;
    struct lua_tlsf_info *tlsf_inf;

  public:
    LuaTLSFService(RTT::TaskContext* tc);
    ~LuaTLSFService();

    bool tlsf_incmem(unsigned int size);
    bool exec_file(const std::string &file);
    bool exec_str(const std::string &str);

    LuaStateHandle getLuaState();
  };
#endif // LUA_RTT_TLSF
} // namespace OCL

#endif // OCL_LUASERVICE_HPP
