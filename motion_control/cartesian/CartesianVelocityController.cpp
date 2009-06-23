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

#include "CartesianVelocityController.hpp"
#include <kdl/kinfam_io.hpp>
#include <kdl/chainiksolvervel_pinv_givens.hpp>
#include <kdl/chainiksolvervel_pinv.hpp>
#include <kdl/chainfksolverpos_recursive.hpp>

#include <ocl/ComponentLoader.hpp>
ORO_LIST_COMPONENT_TYPE( OCL::CartesianVelocityController );

namespace OCL
{
    using namespace std;
    using namespace RTT;
    using namespace KDL;

    CartesianVelocityController::CartesianVelocityController(string name) :
        TaskContext(name,PreOperational),
        cartpos_port("CartesianSensorPosition",KDL::Frame::Identity()),
        cartvel_port("CartesianOutputVelocity",KDL::Twist::Zero()),
        naxespos_port("nAxesSensorPosition"),
        naxesvel_port("nAxesOutputVelocity"),
        chain_prop("Chain","Kinematic Description of the robot chain"),
        toolframe("ToolLocation","Offset between the robot's end effector and the tool location"),
        kinematics_status(true)
    {
        //Create the ports
        this->ports()->addPort(&cartpos_port);
        this->ports()->addPort(&cartvel_port);
        this->ports()->addPort(&naxespos_port);
        this->ports()->addPort(&naxesvel_port);

        this->properties()->addProperty(&chain_prop);
        this->properties()->addProperty(&toolframe);

    }

    CartesianVelocityController::~CartesianVelocityController()
    {
    }

    bool CartesianVelocityController::configureHook()
    {
        chain = chain_prop;
        chain.addSegment(Segment(Joint(Joint::None),toolframe.value()));

        nj = chain.getNrOfJoints();
        jointpositions=new JntArray(nj);
        jointvelocities=new JntArray(nj);
        naxesposition.resize(nj);
        naxesvelocities.resize(nj);

        fksolver=new ChainFkSolverPos_recursive(chain);
        iksolver=new ChainIkSolverVel_pinv(chain);
        return true;

    }

    void CartesianVelocityController::cleanupHook()
    {
        delete jointpositions;
        delete jointvelocities;
        delete fksolver;
        delete iksolver;
    }

    bool CartesianVelocityController::startHook()
    {
        //Initialize: calculate everything once
        this->updateHook();
        //Check if there were any problems calculating the kinematics
        return kinematics_status>=0;
    }

    void CartesianVelocityController::updateHook()
    {
        //Read out the ports
        naxespos_port.Get(naxesposition);
        cartvel_port.Get(cartvel);

        //Check if the jointpositions-port value as a correct size
        if(nj==naxesposition.size()){

            //copy into KDL-type
            for(unsigned int i=0;i<nj;i++)
                (*jointpositions)(i)=naxesposition[i];

            //Calculate forward position kinematics
            kinematics_status = fksolver->JntToCart(*jointpositions,cartpos);
            //Only set result to port if it was calcuted correctly
            if(kinematics_status>=0)
                cartpos_port.Set(cartpos);
            else
                log(Error)<<"Could not calculate forward kinematics"<<endlog();

            //Calculate inverse velocity kinematics
            kinematics_status = iksolver->CartToJnt(*jointpositions,cartvel,*jointvelocities);
            if(kinematics_status<0){
                SetToZero(*jointvelocities);
                log(Error)<<"Could not calculate inverse kinematics"<<endlog();
            }

            for(unsigned int i=0;i<nj;i++)
                naxesvelocities[i]=(*jointvelocities)(i);

            naxesvel_port.Set(naxesvelocities);
        }
        else
            kinematics_status=-1;
    }

    void CartesianVelocityController::stopHook()
    {
        for(unsigned int i=0;i<nj;i++)
            naxesvelocities[i]=0.0;
        naxesvel_port.Set(naxesvelocities);
    }

}





