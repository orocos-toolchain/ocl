// Copyright  (C)  2008  Ruben Smits <ruben dot smits at mech dot kuleuven dot be>

// Author: Ruben Smits <ruben dot smits at mech dot kuleuven dot be>
// URL: http://www.orocos.org/ocl

// This library is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public
// License as published by the Free Software Foundation; either
// version 2.1 of the License, or (at your option) any later version.

// This library is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
// Lesser General Public License for more details.

// You should have received a copy of the GNU Lesser General Public
// License along with this library; if not, write to the Free Software
// Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA

#ifndef _KUKA361CARTESIAN_VELOCITY_CONTROLLER_HPP_
#define _KUKA361CARTESIAN_VELOCITY_CONTROLLER_HPP_

#include "Kuka361Kinematics.hpp"

#include <rtt/TaskContext.hpp>
#include <rtt/Ports.hpp>

#include <ocl/OCL.hpp>

namespace OCL
{

    class Kuka361CartesianVelocityController : public RTT::TaskContext
    {
    public:
        Kuka361CartesianVelocityController(std::string name);
        ~Kuka361CartesianVelocityController();

        virtual bool configureHook();
        virtual bool startHook();
        virtual void updateHook();
        virtual void stopHook();
        virtual void cleanupHook();

    private:
        Kuka361Kinematics kinematics;

        KDL::JntArray jointpositions,jointvelocities;
        std::vector<double> naxesposition,naxesvelocities;

        KDL::Twist cartvel;
        KDL::Frame cartpos;

        RTT::DataPort<KDL::Frame> cartpos_port;
        RTT::DataPort<KDL::Twist> cartvel_port;
        RTT::DataPort<std::vector<double> > naxespos_port;
        RTT::DataPort<std::vector<double> > naxesvel_port;

        int kinematics_status;

    };
}
#endif



