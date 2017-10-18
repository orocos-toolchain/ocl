#include "LuaStateHandle.hpp"

namespace OCL
{

  LuaStateHandle::LuaStateHandle()
    : L(0), m(0)
  {}

  LuaStateHandle::LuaStateHandle(lua_State *L, RTT::os::MutexInterface &mutex)
    : L(L), m(&mutex)
  {
    m->lock();
  }

  LuaStateHandle::LuaStateHandle(const LuaStateHandle &other)
    : L(other.L), m(other.m)
  {
    // take ownership of the mutex lock
    other.m = 0;
  }

  LuaStateHandle::~LuaStateHandle()
  {
    if (m) m->unlock();
  }

  LuaStateHandle &LuaStateHandle::operator=(const LuaStateHandle &other)
  {
    if (this == &other) return *this;
    if (m) m->unlock();

    this->L = other.L;
    this->m = other.m;

    // take ownership of the mutex lock
    other.m = 0;

    return *this;
  }

  lua_State *LuaStateHandle::get() const
  {
    return L;
  }

} // namespace OCL
