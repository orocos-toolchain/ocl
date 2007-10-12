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

namespace OCL
{
    using namespace std;
    using namespace RTT;
    using namespace KDL;

    CartesianVelocityController::CartesianVelocityController(string name,const KDL::Chain& _chain, 
                                                             KDL::ChainIkSolverVel* _iksolver,
                                                             KDL::ChainFkSolverPos* _fksolver) :
        TaskContext(name),
        chain(_chain),
        fksolver(_fksolver),
        iksolver(_iksolver),
        nj(chain.getNrOfJoints()),
        jointpositions(nj),
        jointvelocities(nj),
        naxesposition(nj),
        naxesvelocities(nj),
        cartpos_port("CartesianSensorPosition",KDL::Frame::Identity()),
        cartvel_port("CartesianOutputVelocity",KDL::Twist::Zero()),
        naxespos_port("nAxesSensorPosition",std::vector<double>(nj,0)),
        naxesvel_port("nAxesOutputVelocity",std::vector<double>(nj,0)),
        kinematics_status(true),own_fk(false),own_ik(false)
    {
        //Create the ports
        this->ports()->addPort(&cartpos_port);
        this->ports()->addPort(&cartvel_port);
        this->ports()->addPort(&naxespos_port);
        this->ports()->addPort(&naxesvel_port);
        
        //If no solvers are given create our own:
        if(iksolver==NULL){
            iksolver=new ChainIkSolverVel_pinv(chain);
            own_ik=true;
        }
        if(fksolver==NULL){
            fksolver=new ChainFkSolverPos_recursive(chain);
            own_fk=true;
        }
        
    }

    CartesianVelocityController::~CartesianVelocityController()
    {
        if (own_fk)
            delete fksolver;
        if (own_ik)
            delete iksolver;
    }
    

    bool CartesianVelocityController::startHook()
    {
        //Initialize: calculate everything once
        this->update();
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
                jointpositions(i)=naxesposition[i];

            //Calculate forward position kinematics
            kinematics_status = fksolver->JntToCart(jointpositions,cartpos);
            //Only set result to port if it was calcuted correctly
            if(kinematics_status>=0)
                cartpos_port.Set(cartpos);
            else
                log(Error)<<"Could not calculate forward kinematics"<<endlog();
            
            //Calculate inverse velocity kinematics
            kinematics_status = iksolver->CartToJnt(jointpositions,cartvel,jointvelocities);
            if(kinematics_status<0){
                SetToZero(jointvelocities);
                log(Error)<<"Could not calculate inverse kinematics"<<endlog();
            }
            
            for(unsigned int i=0;i<nj;i++)
                naxesvelocities[i]=jointvelocities(i);
            
            naxesvel_port.Set(naxesvelocities);
        }
        else
            kinematics_status=-1;
    }
}

                                 
                                 

