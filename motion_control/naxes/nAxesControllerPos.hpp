// $Id: nAxisControllerPos.hpp,v 1.1.1.1 2003/12/02 20:32:06 kgadeyne Exp $
// Copyright (C) 2003 Klaas Gadeyne <klaas.gadeyne@mech.kuleuven.ac.be>
//                    Wim Meeussen  <wim.meeussen@mech.kuleuven.ac.be>
// Copyright (C) 2006-2008 Ruben Smits <ruben.smits@mech.kuleuven.be>
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

#include <rtt/TaskContext.hpp>
#include <rtt/Properties.hpp>
#include <rtt/TimeService.hpp>
#include <rtt/Ports.hpp>
#include <rtt/Command.hpp>

#include <ocl/OCL.hpp>

namespace OCL
{
    /**
     * This component can control the positions of multiple axes. It
     * uses a simple position-feedback to calculate an output
     * velocity.  velocity_out = K_gain * ( position_desired -
     * position_measured)
     *
     * The initial state of the component is PreOperational
     * \ingroup nAxesComponents
     */

    class nAxesControllerPos : public RTT::TaskContext
    {
    public:
        /**
         * Constructor of the class
         *
         * @param name name of the component
         *
         */
        nAxesControllerPos(std::string name);

        virtual ~nAxesControllerPos();
        /**
         * Configures the component, make sure the properties are
         * updated using the OCL::DeploymentComponent or the marshalling
         * interface of the component.
         *
         * The number of axes and the control gains are updated.
         *
         * @return false if the gains property does not match the
         * number of axes, true otherwise
         */
        virtual bool configureHook();
        /**
         * Starts the component
         *
         * @return failes if the input-ports are not ready or the size
         * of the input-ports does not match the number of axes this
         * component is configured for.
         */
        virtual bool startHook();
        /**
         * Updates the output using the control equation, the measured
         * and the desired position
         */
        virtual void updateHook();
        virtual void stopHook();

    private:
        bool startMeasuringOffsets(double treshold_moving, int num_samples);
        bool finishedMeasuringOffsets() const;

        unsigned int        num_axes;

        std::vector<double> p_meas,p_desi,v_out,offset_measurement,gain;
    protected:
        /**
         * Command to measure a velocity offset on the axes. The idea
         * is to keep the robot at the same position. If a velocity
         * output is needed to do this, this is the offset you need on
         * your axes output. The command stops after the measurement.
         *
         * @param treshold_moving duration of the measurement
         * @param num_samples number of samples.
         *
         * @return false if still measurering, true otherwise
         */
        RTT::Command<bool(double,int)>              measureOffset;
        /// The measured positions
        RTT::ReadDataPort< std::vector<double> >    p_meas_port;
        /// The desired positions
        RTT::ReadDataPort< std::vector<double> >    p_desi_port;
        /// The output velocities
        RTT::WriteDataPort< std::vector<double> >   v_out_port;
        ///Attribute containing the calculated offsets after executing
        ///the measureOffset Command
        RTT::Attribute<std::vector<double> > offset_attr;

    private:
        int                       num_samples, num_samples_taken;
        double                    time_sleep;
        RTT::TimeService::ticks   time_begin;
        bool                      is_measuring;
    protected:
        /// The control gain values for the axes.
        RTT::Property< std::vector<double> >  gain_prop;
        /// The number of axes to configure the components with.
        RTT::Property< unsigned int > num_axes_prop;

    }; // class
}//namespace
#endif // __N_AXES_CONTROLLER_POS_H__
