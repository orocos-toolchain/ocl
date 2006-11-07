// $Id: nAxisEffectorVel.hpp,v 1.1.1.1 2003/12/02 20:32:06 kgadeyne Exp $
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

#ifndef __N_AXES_EFFECTOR_VEL_H__
#define __N_AXES_EFFECTOR_VEL_H__

#include <rtt/RTT.hpp>

#include <rtt/TaskContext.hpp>
#include <rtt/Ports.hpp>

#include <ocl/OCL.hpp>

namespace OCL
{
    /**
     * This class implements a TaskContext that reads out the
     * output velocity dataports of OCL::nAxesControllerPos,
     * OCL::nAxesControllerPosVel or OCL::nAxesControllerVel and
     * puts these output values in the driveValue dataports of an
     * nAxesVelocityController. 
     * 
     */

    class nAxesEffectorVel : public RTT::TaskContext
    {
    public:
                /** 
         * The contructor of the class 
         * 
         * @param name name of the Taskcontext
         * @param num_axes number of axes that should be read
         * 
         */
        nAxesEffectorVel(std::string name,unsigned int num_axes);
        virtual ~nAxesEffectorVel();
        
        virtual bool startup();
        virtual void update();
        virtual void shutdown();
    
    private:
        unsigned int                                       _num_axes;
        
        std::vector<double>                                _velocity_out_local;
    protected:
        /// DataPort containing the output velocities, shared with
        /// OCL::nAxesControllerPos, OCL::nAxesControllerPosVel
        /// or OCL::nAxesControllerVel 
        RTT::ReadDataPort< std::vector<double> >           _velocity_out;
        /// vector of dataports which write to the
        /// nAxesVelocityController. Default looks for ports with
        /// names driveValue0, driveValue1, ...
        std::vector<RTT::WriteDataPort<double>*>           _velocity_drives;
    
  }; // class
}//namespace
#endif // __N_AXES_EFFECTOR_VEL_H__
