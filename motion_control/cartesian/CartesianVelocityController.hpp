// Copyright  (C)  2007  Ruben Smits <ruben dot smits at mech dot kuleuven dot be>

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

#ifndef __CARTESIAN_VELOCITY_CONTROLLER_HPP__
#define __CARTESIAN_VELOCITY_CONTROLLER_HPP__

#include <kdl/chain.hpp>
#include <kdl/chainfksolver.hpp>
#include <kdl/chainiksolver.hpp>

#include <rtt/TaskContext.hpp>
#include <rtt/Ports.hpp>

#include <ocl/OCL.hpp>

namespace OCL
{
    /**
     * @file   CartesianVelocityController.hpp
     * @author Ruben Smits
     * @date   Mon Aug 20 11:24:08 2007
     * 
     * @brief  This component is the bridge between a hardware
     * nAxesVelocityController and the Cartesian space control
     * components. It calculates the forward position kinematics and
     * the inverse velocity kinematics for a KDL::Chain, the kinematic
     * algorithms are given by the user.
     * 
     */

    class CartesianVelocityController : public RTT::TaskContext
    {
    public:
        CartesianVelocityController(std::string name);
        ~CartesianVelocityController();
        
        virtual bool configureHook();
        virtual bool startHook();
        virtual void updateHook();
        virtual void stopHook();
        virtual void cleanupHook();
        
    private:
        KDL::Chain chain;
        KDL::ChainFkSolverPos* fksolver;
        KDL::ChainIkSolverVel* iksolver;
        
        unsigned int nj;
        
        KDL::JntArray *jointpositions,*jointvelocities;
        std::vector<double> naxesposition,naxesvelocities;
        
        KDL::Twist    cartvel;
        KDL::Frame    cartpos;
        
        RTT::DataPort<KDL::Frame> cartpos_port;
        RTT::DataPort<KDL::Twist> cartvel_port;
        RTT::DataPort<std::vector<double> > naxespos_port;
        RTT::DataPort<std::vector<double> > naxesvel_port;
        
        RTT::Property<std::string> chain_location;
        RTT::Property<KDL::Frame> toolframe;
        
        bool kinematics_status;
        
    };
}
#endif
    
        
        
