// $Id: nAxisControllerVel.hpp,v 1.1.1.1 2003/12/02 20:32:06 kgadeyne Exp $
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

#ifndef __N_AXES_CONTROLLER_VEL_H__
#define __N_AXES_CONTROLLER_VEL_H__

#include <rtt/RTT.hpp>
#include <rtt/TaskContext.hpp>
#include <rtt/Properties.hpp>
#include <rtt/TimeService.hpp>
#include <rtt/Ports.hpp>

#include <ocl/OCL.hpp>

namespace OCL
{
    /**
     * This class implements a TaskContext that controlls the
     * velocities of an axis. It uses a simple position-feedback
     * and velocity feedforward to calculate an output velocity. 
     * velocity_out = K_gain * ( position_desired - position_measured)
     * + velocity_desired.
     * The desired position is calculated by integrating the desired velocity
     * 
     * \ingroup nAxesComponents 
     */
  
    class nAxesControllerVel : public RTT::TaskContext
    {
    public:
        /** 
         * Constructor of the class
         * 
         * @param name name of the TaskContext
         * 
         */
        nAxesControllerVel(std::string name);
        virtual ~nAxesControllerVel();
        
        virtual bool configureHook();
        virtual bool startHook();
        virtual void updateHook();
        virtual void stopHook();
        
    private:
        void resetAll();
        void resetAxis(int axis);
        
        unsigned int  num_axes;
  					       
        std::vector<double>  p_m, p_d, v_d, v_out, gain;
    protected:
        /// Method to reset the controller of all axes
        RTT::Method<void(void)>  reset_all_mtd;
        /**
         * Method to reset the controller of axis
         * 
         * @param axis axis controller to be reset
         */
        RTT::Method<void(int)>   resetAxis_mtd;
        
        /// DataPort containing the measured positions, shared with
        /// OCL::nAxesSensor 
        RTT::ReadDataPort< std::vector<double> >   p_m_port;
        /// DataPort containing the desired velocities, shared with
        /// OCL::nAxesGeneratorVel
        RTT::ReadDataPort< std::vector<double> >   v_d_port;
        /// DataPort containing the output velocities, shared with
        /// OCL::nAxesEffectorVel 
        RTT::WriteDataPort< std::vector<double> >  v_out_port;
        /// Vector with the control gain value for each axis.
        RTT::Property< std::vector<double> >       gain_prop;
        RTT::Property<unsigned int> num_axes_prop;
        
    private:
        std::vector<bool>                          is_initialized;
        std::vector<RTT::TimeService::ticks>       time_begin;
        
    
  }; // class
}//namespace

#endif // __N_AXES_CONTROLLER_VEL_H__
