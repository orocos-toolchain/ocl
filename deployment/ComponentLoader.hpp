#ifndef OCL_COMPONENT_LOADER_HPP
#define OCL_COMPONENT_LOADER_HPP

#include <map>
#include <string>

namespace RTT {
  class TaskContext;
}

namespace OCL
{
    /**
     * This signature defines how a component can be instantiated.
     */
    typedef RTT::TaskContext* (*ComponentLoaderSignature)(std::string instance_name);
}

namespace OCL
{
    class ComponentFactories
    {
    public:
        /**
         * When static linking is used, the Component library loaders can be
         * found in this map.
         */
        static std::map<std::string,ComponentLoaderSignature> Factories;
    };

    template<class C>
    class ComponentLoader
    {
    public:
        ComponentLoader(std::string type_name)
        {
            ComponentFactories::Factories[type_name] = &ComponentLoader<C>::createComponent;
        }

        static RTT::TaskContext* createComponent(std::string instance_name)
        {
            return new C(instance_name);
        }
    };
}

#define COMPONENT_LOADER(CNAME) \
namespace {\
  ComponentLoader<CNAME> static_loader("CNAME"); \
} \
extern "C" { \
  RTT::TaskContext* createComponent(std::string instance_name) \
  { \
    return new CNAME(instance_name); \
  } \
}

#endif
