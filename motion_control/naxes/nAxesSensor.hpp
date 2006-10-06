// $Id: nAxesSensorPos.hpp,v 1.1.1.1 2003/12/02 20:32:06 kgadeyne Exp $
// Copyright (C) 2003 Klaas Gadeyne <klaas.gadeyne@mech.kuleuven.ac.be>
//                    Wim Meeussen  <wim.meeussen@mech.kuleuven.ac.be>
// Copyright (C) 2006 Ruben Smits <ruben.smits@mech.kuleuven.be>
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

#ifndef __N_AXES_SENSOR_POS_H__
#define __N_AXES_SENSOR_POS_H__

#include <rtt/RTT.hpp>
#include <rtt/TaskContext.hpp>
#include <rtt/Ports.hpp>

namespace Orocos
{
    /**
     * This class implements a TaskContext that reads out the
     * positionValue and driveValue dataports of a
     * nAxesVelocityController. It collects these values and stores
     * them as a vector in a dataport, available for other nAxes or
     * your own components.
     * 
     */
    class nAxesSensor : public RTT::TaskContext
    {
    public:
        /** 
         * The contructor of the class 
         * 
         * @param name name of the Taskcontext
         * @param num_axes number of axes that should be read
         * 
         */
        nAxesSensor(std::string name,unsigned int num_axes);
    
        virtual ~nAxesSensor();
  
        // Redefining virtual members
        virtual bool startup();
        virtual void update();
        virtual void shutdown();
  
    protected:
        unsigned int                                _num_axes;
        std::vector<double>                         _position_local;
        std::vector<double>                         _velocity_local;
        /// vector of dataports which read from the
        /// nAxesVelocityController. Default looks for ports with
        /// names positionValue0, positionValue1, ...
        std::vector< RTT::ReadDataPort<double>* >   _position_sensors;
        /// vector of dataports which read from the
        /// nAxesVelocityController. Default looks for ports with
        /// names driveValue0, driveValue1, ...
        std::vector< RTT::ReadDataPort<double>* >   _velocity_sensors;
        /// Dataport with a vector that contains the collected the
        /// position values 
        RTT::WriteDataPort< std::vector<double> >   _position_naxes;
        /// Dataport with a vector that contains the collected the
        /// drive/velocity values 
        RTT::WriteDataPort< std::vector<double> >   _velocity_naxes;
        
    }; // class
}//namespace
#endif // __N_AXES_SENSOR_POS_H__
