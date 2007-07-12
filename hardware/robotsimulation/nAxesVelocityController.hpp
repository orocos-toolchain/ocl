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

#ifndef SIMULATION_NAXES_VELOCITY_CONTROLLER_HPP
#define SIMULATION_NAXES_VELOCITY_CONTROLLER_HPP

#include <rtt/RTT.hpp>

#include <rtt/TaskContext.hpp>
#include <rtt/Ports.hpp>
#include <rtt/Event.hpp>
#include <rtt/Method.hpp>
#include <rtt/Properties.hpp>

#include "dev/SimulationAxis.hpp"
#include <rtt/dev/AxisInterface.hpp>

#include <ocl/OCL.hpp>

namespace OCL
{
    /**
     * This class implements a Taskcontext that simulates a robot with
     * a configurable number of axes.
     * 
     */

    class nAxesVelocityController: public RTT::TaskContext
    {
    public:
        nAxesVelocityController(std::string name,unsigned int nrofjoints, std::string propertyfile="cpf/nAxesVelocityController.cpf");
        virtual ~nAxesVelocityController(){};
    
    private:
        virtual bool startAxes();
        virtual bool stopAxes();
        virtual bool lockAxes();
        virtual bool unlockAxes();
        
        virtual bool configureHook();
        virtual bool startHook();
        virtual void updateHook();
        virtual void cleanupHook();
        virtual void stopHook();
        
        const std::string propertyfile;
        
        unsigned int naxes;
        
        std::vector<double> driveValues;
        std::vector<double> positionValues;    
        
        std::vector<RTT::SimulationAxis*> simulation_axes;

    protected:
        Method<bool(void)> M_startAxes;
        Method<bool(void)> M_stopAxes;
        Method<bool(void)> M_unlockAxes;
        Method<bool(void)> M_lockAxes; 
        
        RTT::DataPort<std::vector<double> > D_driveValues;
        RTT::DataPort<std::vector<double> > D_positionValues;
        
        RTT::Property<std::vector<double> > P_initialPositions;
        
        RTT::Constant<unsigned int> A_naxes;
        
    };
}
#endif
