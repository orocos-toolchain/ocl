// Copyright (C) 2007 Ruben Smits <ruben dot smits at mech dot kuleuven dot be>
//  
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2 of the License, or
// (at your option) any later version.
//  
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//  
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
//  

#include "nAxesVelocityController.hpp"
#include <ocl/ComponentLoader.hpp>
#include <rtt/Logger.hpp>

namespace OCL
{
    using namespace RTT;
    using namespace std;
    
    nAxesVelocityController::nAxesVelocityController(string name)
        : TaskContext(name,PreOperational),
          M_startAllAxes("startAllAxes",&nAxesVelocityController::startAllAxes,this),
          M_stopAllAxes("stopAllAxes",&nAxesVelocityController::stopAllAxes,this),
          M_unlockAllAxes("unlockAllAxes",&nAxesVelocityController::unlockAllAxes,this),
          M_lockAllAxes("lockAllAxes",&nAxesVelocityController::lockAllAxes,this),
          D_driveValues("nAxesOutputVelocity"),
          D_positionValues("nAxesSensorPosition"),
          P_initialPositions("initialPositions","initial positions (rad) for the axes"),
          P_naxes("naxes","number of axes")
    {
        methods()->addMethod(&M_startAllAxes,"start the simulation axes");
        methods()->addMethod(&M_stopAllAxes,"stop the simulation axes");
        methods()->addMethod(&M_lockAllAxes,"lock the simulation axes");
        methods()->addMethod(&M_unlockAllAxes,"unlock the simulation axes");
        
        ports()->addPort(&D_driveValues);
        ports()->addPort(&D_positionValues);
        
        properties()->addProperty(&P_initialPositions);
        properties()->addProperty(&P_naxes);
        
    }
    
    bool nAxesVelocityController::configureHook()
    {
        naxes=P_naxes.value();
        if(P_initialPositions.value().size()!=naxes){
            Logger::In in(this->getName().data());
            log(Error)<<"Size of "<<P_initialPositions.getName()
                      <<" does not match "<<P_naxes.getName()
                      <<endlog();
            return false;
        }
        
        driveValues.resize(naxes);
        positionValues=P_initialPositions.value();
        
        for (unsigned int i = 0; i <naxes; i++){
            simulation_axes[i] = new SimulationAxis(positionValues[i]);
        }

        D_positionValues.Set(positionValues);
        
        return true;
    }

    bool nAxesVelocityController::startHook()
    {
        //check connection and sizes of input-ports
        if(!D_driveValues.ready()){
            Logger::In in(this->getName().data());
            log(Error)<<D_driveValues.getName()<<" not ready"<<endlog();
            return false;
        }
        if(D_driveValues.Get().size()!=naxes){
            Logger::In in(this->getName().data());
            log(Error)<<"Size of "<<D_driveValues.getName()<<": "<<D_driveValues.Get().size()<<" != " << naxes<<endlog();
            return false;
        }
        
        return true;
    }
    
    void nAxesVelocityController::updateHook()
    {
        D_driveValues.Get(driveValues);
        for (unsigned int i=0;i<naxes;i++){
            positionValues[i]=simulation_axes[i]->getSensor("Position")->readSensor();
            if(simulation_axes[i]->isDriven())
                simulation_axes[i]->drive(driveValues[i]);
        }
        D_positionValues.Set(positionValues);
    }
    
    void nAxesVelocityController::cleanupHook()
    {
        simulation_axes.clear();
    }
    
    void nAxesVelocityController::stopHook()
    {
    }
    
    bool nAxesVelocityController::startAllAxes()
    {
        bool retval=true;
        for(vector<SimulationAxis*>::iterator axis=simulation_axes.begin();
            axis!=simulation_axes.end();axis++)
            retval&=(*axis)->drive(0.0);
        return retval;
    }

    bool nAxesVelocityController::stopAllAxes()
    {
            bool retval=true;
        for(vector<SimulationAxis*>::iterator axis=simulation_axes.begin();
            axis!=simulation_axes.end();axis++)
            retval&=(*axis)->stop();
        return retval;
    }

    bool nAxesVelocityController::lockAllAxes()
    {
    bool retval=true;
        for(vector<SimulationAxis*>::iterator axis=simulation_axes.begin();
            axis!=simulation_axes.end();axis++)
            retval&=(*axis)->lock();
        return retval;
        
    }

    bool nAxesVelocityController::unlockAllAxes()
    {
    bool retval=true;
        for(vector<SimulationAxis*>::iterator axis=simulation_axes.begin();
            axis!=simulation_axes.end();axis++)
            retval&=(*axis)->unlock();
        return retval;
    }
}
ORO_CREATE_COMPONENT(OCL::nAxesVelocityController)
        
          
