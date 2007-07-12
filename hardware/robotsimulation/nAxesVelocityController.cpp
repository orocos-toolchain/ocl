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

#include <rtt/Logger.hpp>

namespace OCL
{
    using namespace RTT;
    using namespace std;
    
    nAxesVelocityController::nAxesVelocityController(string name, unsigned int _naxes, string _propertyfile)
        : TaskContext(name,PreOperational),
          propertyfile(_propertyfile),
          naxes(_naxes),
          driveValues(naxes,0),
          positionValues(naxes,0),
          simulation_axes(naxes),
          M_startAxes("startAxes",&nAxesVelocityController::startAxes,this),
          M_stopAxes("stopAxes",&nAxesVelocityController::stopAxes,this),
          M_unlockAxes("unlockAxes",&nAxesVelocityController::unlockAxes,this),
          M_lockAxes("lockAxes",&nAxesVelocityController::lockAxes,this),
          D_driveValues("nAxesOutputVelocity",driveValues),
          D_positionValues("nAxesSensorPosition",positionValues),
          P_initialPositions("initialPositions","initial positions (rad) for the axes",positionValues),
          A_naxes("naxes",naxes)
    {
        methods()->addMethod(&M_startAxes,"start the simulation axes");
        methods()->addMethod(&M_stopAxes,"stop the simulation axes");
        methods()->addMethod(&M_lockAxes,"lock the simulation axes");
        methods()->addMethod(&M_unlockAxes,"unlock the simulation axes");
        
        ports()->addPort(&D_driveValues);
        ports()->addPort(&D_positionValues);
        
        properties()->addProperty(&P_initialPositions);
        attributes()->addAttribute(&A_naxes);
        
    }
    
    bool nAxesVelocityController::configureHook()
    {
        this->marshalling()->readProperties(propertyfile);
        
        for (unsigned int i = 0; i <naxes; i++){
            simulation_axes[i] = new SimulationAxis(P_initialPositions.value()[i]);
        }
        return true;
    }

    bool nAxesVelocityController::startHook()
    {
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
    
    bool nAxesVelocityController::startAxes()
    {
        bool retval=true;
        for(vector<SimulationAxis*>::iterator axis=simulation_axes.begin();
            axis!=simulation_axes.end();axis++)
            retval&=(*axis)->drive(0.0);
        return retval;
    }

    bool nAxesVelocityController::stopAxes()
    {
            bool retval=true;
        for(vector<SimulationAxis*>::iterator axis=simulation_axes.begin();
            axis!=simulation_axes.end();axis++)
            retval&=(*axis)->stop();
        return retval;
    }

    bool nAxesVelocityController::lockAxes()
    {
    bool retval=true;
        for(vector<SimulationAxis*>::iterator axis=simulation_axes.begin();
            axis!=simulation_axes.end();axis++)
            retval&=(*axis)->lock();
        return retval;
        
    }

    bool nAxesVelocityController::unlockAxes()
    {
    bool retval=true;
        for(vector<SimulationAxis*>::iterator axis=simulation_axes.begin();
            axis!=simulation_axes.end();axis++)
            retval&=(*axis)->unlock();
        return retval;
    
    }
}

        
          
