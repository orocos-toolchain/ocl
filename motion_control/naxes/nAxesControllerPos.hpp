// $Id: nAxisControllerPos.hpp,v 1.1.1.1 2003/12/02 20:32:06 kgadeyne Exp $
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

#ifndef __N_AXES_CONTROLLER_POS_H__
#define __N_AXES_CONTROLLER_POS_H__

#include <rtt/RTT.hpp>

#include <rtt/GenericTaskContext.hpp>
#include <rtt/Properties.hpp>
#include <rtt/TimeService.hpp>
#include <rtt/Ports.hpp>
#include <rtt/Command.hpp>

namespace Orocos
{
    /**
     * This class implements a TaskContext that controlls the
     * positionValues of an axis. It uses a simple position-feedback
     * to calculate an output velocity. 
     * velocity_out = K_gain * ( position_desired - position_measured)
     * It can share dataports with
     * Orocos::nAxesSensor to get the measured positions, with
     * Orocos::nAxesGeneratorPos to get its desired positions and with
     * Orocos::nAxesEffectorVel to send its output velocities to the
     * hardware/simulation axes.
     * 
     */

    class nAxesControllerPos : public RTT::GenericTaskContext
    {
    public:
        /** 
         * Constructor of the class
         * 
         * @param name name of the TaskContext
         * @param num_axes number of axes
         * @param propertyfile location of the propertyfile. Default:
         * cpf/nAxesControllerPos.cpf 
         * 
         */
        nAxesControllerPos(std::string name, unsigned int num_axes,
                           std::string propertyfile="cpf/nAxesControllerPos.cpf");
        
        virtual ~nAxesControllerPos();
        
        virtual bool startup();
        virtual void update();
        virtual void shutdown();
  
    private:
        bool startMeasuringOffsets(double treshold_moving, int num_samples);
        bool finishedMeasuringOffsets() const;
        const std::vector<double>& getMeasurementOffsets();
    
        unsigned int                                _num_axes;
        std::string                                 _propertyfile;
        
        std::vector<double>                         _position_meas_local, _position_desi_local, _velocity_out_local, _offset_measurement;
    protected:
        /** 
         * Command to measure a velocity offset on the axes. The idea
         * is to keep the robot at the same position. If a velocity
         * output is needed to do this, this is the offset you need on you
         * axes output. The command stops after the measurement
         * 
         * @param treshold_moving duration of the measurement
         * @param num_samples number of samples.
         * 
         * @return false if still measurering, true otherwise
         */
        RTT::Command<bool(double,int)>              _measureOffset;
        /** 
         * Method to get the calculated offsets. Use it only after the
         *nAxesControllerPos::_measureOffset command.
         * 
         * 
         * @return vector with the offset value for each axis.
         */
        RTT::Method<std::vector<double>(void)>      _getOffset;
        /// DataPort containing the measured positions, shared with
        /// Orocos::nAxesSensor 
        RTT::ReadDataPort< std::vector<double> >    _position_meas;
        /// DataPort containing the desired positions, shared with
        /// Orocos::nAxesGeneratorPos 
        RTT::ReadDataPort< std::vector<double> >    _position_desi;
        /// DataPort containing the output velocities, shared with
        /// Orocos::nAxesEffectorVel 
        RTT::WriteDataPort< std::vector<double> >   _velocity_out;
    private:
        int                                         _num_samples, _num_samples_taken;
        double                                      _time_sleep;
        RTT::TimeService::ticks                     _time_begin;
        bool                                        _is_measuring;
    protected:
        /// Vector with the control gain value for each axis.
        RTT::Property< std::vector<double> >        _controller_gain;
        
    }; // class
}//namespace
#endif // __N_AXES_CONTROLLER_POS_H__
