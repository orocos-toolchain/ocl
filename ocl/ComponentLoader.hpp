/***************************************************************************
  tag: Peter Soetens  Thu Jul 3 15:35:28 CEST 2008  ComponentLoader.hpp 

                        ComponentLoader.hpp -  description
                           -------------------
    begin                : Thu July 03 2008
    copyright            : (C) 2008 Peter Soetens
    email                : peter.soetens@fmtc.be
 
 ***************************************************************************
 *   This library is free software; you can redistribute it and/or         *
 *   modify it under the terms of the GNU Lesser General Public            *
 *   License as published by the Free Software Foundation; either          *
 *   version 2.1 of the License, or (at your option) any later version.    *
 *                                                                         *
 *   This library is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU     *
 *   Lesser General Public License for more details.                       *
 *                                                                         *
 *   You should have received a copy of the GNU Lesser General Public      *
 *   License along with this library; if not, write to the Free Software   *
 *   Foundation, Inc., 59 Temple Place,                                    *
 *   Suite 330, Boston, MA  02111-1307  USA                                *
 *                                                                         *
 ***************************************************************************/
 
 
#ifndef OCL_COMPONENT_LOADER_HPP
#define OCL_COMPONENT_LOADER_HPP

#include <string>
#include <map>
#include <vector>
#include <ocl/OCL.hpp>

namespace RTT {
  class TaskContext;
}

namespace OCL
{
    /**
     * This signature defines how a component can be instantiated.
     */
    typedef RTT::TaskContext* (*ComponentLoaderSignature)(std::string instance_name);
    typedef std::map<std::string,ComponentLoaderSignature> FactoryMap;

    /**
     * A global variable storing all component factories added with
     * \a ORO_LIST_COMPONENT_TYPE.
     */
    class ComponentFactories
    {
        /**
         * When static linking is used, the Component library loaders can be
         * found in this map.
         */
        OCL_HIDE static FactoryMap* Factories;
    public:
        OCL_HIDE static FactoryMap& Instance() {
            if ( Factories == 0)
                Factories = new FactoryMap();
            return *Factories;
        }
    };

    /**
     * A helper class storing a single component factory.
     */
    template<class C>
    class ComponentLoader
    {
    public:
        ComponentLoader(std::string type_name)
        {
            ComponentFactories::Instance()[type_name] = &ComponentLoader<C>::createComponent;
        }

        static RTT::TaskContext* createComponent(std::string instance_name)
        {
            return new C(instance_name);
        }
    };
}

// Helper macros.
#define ORO_CONCAT_LINE2(x,y) x##y 
#define ORO_CONCAT_LINE1(x,y) ORO_CONCAT_LINE2(x,y) 
#define ORO_CONCAT_LINE(x) ORO_CONCAT_LINE1(x,__LINE__) 
 
#define ORO_LIST_COMPONENT_TYPE_str(s) ORO_LIST_COMPONENT_TYPE__str(s)
#define ORO_LIST_COMPONENT_TYPE__str(s) #s

// ORO_CREATE_COMPONENT and ORO_CREATE_COMPONENT_TYPE are only used in shared libraries.
#if defined(OCL_DLL_EXPORT)

/**
 * Use this macro to register a single component in a shared library (plug-in).
 * You can only use this macro once in a .cpp file for the whole shared library.
 * It adds a function 'createComponent', which will return a new instance of
 * the library's component type and a function 'getComponentType', which returns
 * the type (namespace::class) name of the component.
 *
 * The advantage of this approach is that the user does not need to know the
 * class name of the component, he just needs to locate the shared library itself.
 * The disadvantage is that only one component \a type per shared library can be created.
 *
 * @param CNAME the class name of the component you are adding to the library.
 */
#define ORO_CREATE_COMPONENT(CNAME) \
extern "C" { \
  OCL_API RTT::TaskContext* createComponent(std::string instance_name) \
  { \
    return new CNAME(instance_name); \
  } \
  OCL_API std::string getComponentType() \
  { \
    return ORO_LIST_COMPONENT_TYPE_str(CNAME); \
  } \
} /* extern "C" */

/**
 * Use this macro to export all components listed with
 * ORO_LIST_COMPONENT_TYPE in a shared library (plug-in).  It will add
 * a C function 'createComponentType' which can create a component of
 * each class added with ORO_LIST_COMPONENT_TYPE.
 */
#define ORO_CREATE_COMPONENT_TYPE() \
OCL::FactoryMap* OCL::ComponentFactories::Factories = 0; \
extern "C" { \
  OCL_API RTT::TaskContext* createComponentType(std::string instance_name, std::string type_name) \
  { \
    if( OCL::ComponentFactories::Instance().count(type_name) ) \
      return OCL::ComponentFactories::Instance()[type_name](instance_name); \
    return 0; \
  } \
  OCL_API std::vector<std::string> getComponentTypeNames() \
  { \
    std::vector<std::string> ret; \
    OCL::FactoryMap::iterator it; \
    for(it = OCL::ComponentFactories::Instance().begin(); it != OCL::ComponentFactories::Instance().end(); ++it) { \
        ret.push_back(it->first); \
    } \
    return ret; \
  } \
  OCL_API OCL::FactoryMap* getComponentFactoryMap() { return &OCL::ComponentFactories::Instance(); } \
} /* extern "C" */

#else

// Static OCL library:
// Identical to ORO_LIST_COMPONENT_TYPE:
#define ORO_CREATE_COMPONENT(CLASS_NAME) namespace { namespace ORO_CONCAT_LINE(LOADER_) { { OCL::ComponentLoader<CLASS_NAME> m_cloader(ORO_LIST_COMPONENT_TYPE_str(CLASS_NAME)); } }
#define ORO_CREATE_COMPONENT_TYPE() __attribute__((weak)) OCL::FactoryMap* OCL::ComponentFactories::Factories = 0;

#endif

/**
 * Use this macro to register multiple components in a shared library (plug-in).
 * For each component, add this line in the .cpp file. Use this macro in combination with
 * ORO_CREATE_COMPONENT_TYPE.
 *
 * The advantage of this approach is that one library can create different component
 * \a types.
 * The disadvantage is that the user needs to know the component types which are in the shared library.
 * You can get the component types of a library by calling \a getComponentTypeNames().
 *
 * This macro can be used for both shared and static libraries. In case of a shared library,
 * the component factory will be registered to the shared library's local FactoryMap. In case
 * of a static library, the component factory will be registered in the static library's global
 * FactoryMap. In both cases, the DeploymentComponent can access these factories and
 * create the registered component types.
 * 
 * @param CLASS_NAME the class name of the component you are adding to the library.
 */

#define ORO_LIST_COMPONENT_TYPE(CLASS_NAME) namespace { namespace ORO_CONCAT_LINE(LOADER_) { OCL::ComponentLoader<CLASS_NAME> m_cloader(ORO_LIST_COMPONENT_TYPE_str(CLASS_NAME)); } }


#endif

//#undef ORO_CLOADER_CONCAT
