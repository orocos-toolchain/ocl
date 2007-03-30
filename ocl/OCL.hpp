#ifndef ORO_OCL_HPP
#define ORO_OCL_HPP

#include <rtt/rtt-config.h>

namespace RTT {}

/**
 * The Orocos Component Library.
 * This namespace contains components for supporting
 * applications, such as the TaskBrowser, DeploymentComponent,
 * ReportingComponent,... hardware access such as the
 * IOComponent, AxesComponent, Kuka361Component,... or
 * higher level application logic such as the CartesianControllerVel
 * or nAxesGeneratorPos.
 *
 * @note
 * Including this header makes all the classes of the RTT namespace
 * available in the OCL namespace.
 * Thus a component written in the OCL namespace does not need
 * to write the repetitive RTT:: scope.
 */
namespace OCL
{
    // all components are built upon the RTT.
    using namespace RTT;
}

namespace Orocos
{
    using namespace OCL;
}

// This is only an advantage if the OCL is compiled with -fvisibility=hidden.
// The OCL_DLL_EXPORT flag may only be set when OCL itself is compiled and *not*
// after installation.
#if defined(__GNUG__) && defined(__unix__) && defined(RTT_GCC_HASVISIBILITY)
# if defined(OCL_DLL_EXPORT)
#  define OCL_API    __attribute__((visibility("default")))
#  define OCL_EXPORT __attribute__((visibility("default")))
#  define OCL_HIDE   __attribute__((visibility("hidden")))
# else
#  define OCL_API
#  define OCL_EXPORT __attribute__((visibility("default")))
#  define OCL_HIDE   __attribute__((visibility("hidden")))
# endif
#else
# define OCL_API
# define OCL_EXPORT
# define OCL_HIDE
#endif


#endif
