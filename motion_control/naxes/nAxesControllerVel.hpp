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
     * It can share dataports with
     * OCL::nAxesSensor to get the measured positions, with
     * OCL::nAxesGeneratorVel to get its desired velocities and with
     * OCL::nAxesEffectorVel to send its output velocities to the
     * hardware/simulation axes.
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
         * @param num_axes number of axes
         * @param propertyfile location of the propertyfile. Default:
         * cpf/nAxesControllerPosVel.cpf 
         * 
         */
        nAxesControllerVel(std::string name,unsigned int num_axes, 
                           std::string propertyfile="cpf/nAxesControllerVel.cpf");
        virtual ~nAxesControllerVel();
        
        virtual bool startup();
        virtual void update();
        virtual void shutdown();
        
    private:
        void resetAll();
        void resetAxis(int axis);
        
        unsigned int                               _num_axes;
        std::string                                _propertyfile;
  					       
        std::vector<double>                        _position_meas_local, _position_desi_local, _velocity_desi_local, _velocity_out_local;
    protected:
        /// Method to reset the controller of all axes
        RTT::Method<void(void)>                    _reset_all;
        /**
         * Method to reset the controller of axis
         * 
         * @param axis axis controller to be reset
         */
        RTT::Method<void(int)>                     _resetAxis;
        
        /// DataPort containing the measured positions, shared with
        /// OCL::nAxesSensor 
        RTT::ReadDataPort< std::vector<double> >   _position_meas;
        /// DataPort containing the desired velocities, shared with
        /// OCL::nAxesGeneratorVel
        RTT::ReadDataPort< std::vector<double> >   _velocity_desi;
        /// DataPort containing the output velocities, shared with
        /// OCL::nAxesEffectorVel 
        RTT::WriteDataPort< std::vector<double> >  _velocity_out;
        /// Vector with the control gain value for each axis.
        RTT::Property< std::vector<double> >       _controller_gain;
        
    private:
        std::vector<bool>                          _is_initialized;
        std::vector<RTT::TimeService::ticks>       _time_begin;
        
    
  }; // class
}//namespace

#endif // __N_AXES_CONTROLLER_VEL_H__
