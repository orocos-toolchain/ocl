#ifndef OCL_LUASTATEHANDLE_HPP
#define OCL_LUASTATEHANDLE_HPP

#include <rtt/os/MutexLock.hpp>

struct lua_State;

namespace OCL
{
  class LuaStateHandle
  {
  private:
    lua_State *L;
    mutable RTT::os::MutexInterface *m;

  public:
    LuaStateHandle();
    LuaStateHandle(const LuaStateHandle &);
    LuaStateHandle(lua_State *L, RTT::os::MutexInterface &mutex);
    ~LuaStateHandle();

    LuaStateHandle &operator=(const LuaStateHandle &);

    lua_State *get() const;
    operator lua_State *() const { return get(); }
    lua_State *operator->() const { return get(); }

  };

} // namespace OCL

#endif // OCL_LUASTATEHANDLE_HPP
