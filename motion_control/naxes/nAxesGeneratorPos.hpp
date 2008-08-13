// $Id: nAxisGeneratorPos.hpp,v 1.1.1.1 2003/12/02 20:32:06 kgadeyne Exp $
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

#ifndef __N_AXES_GENERATOR_POS_H__
#define __N_AXES_GENERATOR_POS_H__

#include <rtt/RTT.hpp>

#include <rtt/TaskContext.hpp>
#include <rtt/Properties.hpp>
#include <rtt/Ports.hpp>
#include <rtt/Command.hpp>
#include <rtt/Method.hpp>

#include <kdl/velocityprofile_trap.hpp>
#include <rtt/TimeService.hpp>

#include <ocl/OCL.hpp>

namespace OCL
{
    /**
     * This component generates paths between the current positions
     * and new desired positions for multiple axes. It uses KDL for
     * the time interpolation. The paths of all axes are
     * synchronised, this means that all axes movements are scaled in
     * time to the longest axis-movement. The interpolation uses a
     * trapezoidal velocity profile using a maximum acceleration and a
     * maximum velocity.
     *
     * The initial state of the component is PreOperational.
     * \ingroup nAxesComponents
     */

    class nAxesGeneratorPos : public RTT::TaskContext
    {
    public:
        /**
         * Constructor of the class.
         * The interface of the component is build.
         * @param name name of the TaskContext
         */
        nAxesGeneratorPos(std::string name);
        virtual ~nAxesGeneratorPos();

        /**
         * Configures the component, make sure the properties are
         * updated using the OCL::DeploymentComponent or the marshalling
         * interface of the component.
         *
         * The number of axes, maximum velocities and accelerations
         * for the profiles are updated.
         *
         * @return false if the velocities and accelerations
         * properties do not match the number of axes, true otherwise
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
         * Updates the desired position and velocity according to the
         * calculated profile.
         */
        virtual void updateHook();

        virtual void stopHook();

    private:
        bool moveTo(const std::vector<double>& position, double time=0);
        bool moveFinished() const;
        void resetPosition();

        unsigned int num_axes;
        std::vector<double> p_m, p_d, v_d, v_max, a_max;
    protected:
        /**
         * Command that generates a new motion-profile starting from
         * the current time. The command is completed when the
         * duration is passed.
         *
         * @param position a vector with the desired positions of the
         * axes
         * @param time the minimum time duration of the movement, if
         * zero the movement will be as fast as possible.
         *
         * @return false if another motion is still going on or the
         * size of the position parameter does not match the number of
         * axes the component is configured for, true otherwise.
         */
        RTT::Command<bool(std::vector<double>,double)> moveTo_cmd;
        /**
         * Method that resets the generators desired position to the
         * current measured position and the desired velocity to zero
         */
        RTT::Method<void(void)> reset_position_mtd;
        /// DataPort containing the current measured position
        RTT::ReadDataPort< std::vector<double> >  p_m_port;
        /// DataPort containing the current desired position.
        RTT::WriteDataPort< std::vector<double> > p_d_port;
        /// DataPort containing the current desired velocity.
        RTT::WriteDataPort< std::vector<double> > v_d_port;
    private:
        std::vector<KDL::VelocityProfile_Trap>    motion_profile;
        RTT::TimeService::ticks                   time_begin;
        RTT::TimeService::Seconds                 time_passed;
        double                                    max_duration;

        bool                                      is_moving;
    protected:
        /// Vector with the maximum velocity of each axis
        RTT::Property< std::vector<double> >      v_max_prop;
        /// Vector with the maximum acceleration of each axis
        RTT::Property< std::vector<double> >      a_max_prop;
        /// Number of axes to configure the component for
        RTT::Property< unsigned int > num_axes_prop;
    }; // class
}//namespace
#endif // __N_AXES_GENERATOR_POS_H__
