#ifndef ORO_OCL_HPP
#define ORO_OCL_HPP

#include <rtt/RTT.hpp>
#include "ocl-config.h"

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

#endif
