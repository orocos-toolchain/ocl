// Copyright  (C)  2007  Ruben Smits <ruben dot smits at mech dot kuleuven dot be>

// Version: 1.0
// Author: Ruben Smits <ruben dot smits at mech dot kuleuven dot be>
// Maintainer: Ruben Smits <ruben dot smits at mech dot kuleuven dot be>
// URL: http://www.orocos.org/ocl

// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2 of the License, or
// (at your option) any later version.

// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.

// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA

#ifndef WRENCH_SENSOR_H
#define WRENCH_SENSOR_H

#include <rtt/TaskContext.hpp>
#include <rtt/Attribute.hpp>
#include <rtt/Event.hpp>
#include <rtt/Method.hpp>
#include <rtt/Command.hpp>
#include <rtt/Ports.hpp>
#include <kdl/bindings/rtt/toolkit.hpp>
#include <kdl/frames.hpp>

#include <rtt/RTT.hpp>
#if defined (OROPKG_OS_LXRT)

#include "drivers/jr3_lxrt_common.h"

#else

struct s16Forces
{
    short   Fx, Fy, Fz, Tx, Ty, Tz; // Signed 16 bit
};

#endif

#include <ocl/OCL.hpp>

namespace OCL
{
    /**
     * This class implements a TaskContext that communicates with a
     * JR3-WrenchSensor. It should be executed by a periodic activity.
     * Real sensor information is only supplied if the activity is
     * running in lxrt and the jr3-driver is succesfully loaded.
     */

    class WrenchSensor : public RTT::TaskContext
    {
    public:
        WrenchSensor(std::string name);
        virtual ~WrenchSensor(){};

    protected:
        virtual bool configureHook();
        virtual bool startHook();
        virtual void updateHook();
        virtual void stopHook();
        virtual void cleanupHook(){};

        /// DataPort which contains Wrench information
        RTT::DataPort<KDL::Wrench> outdatPort;

        /// Event that is fired if the measured force exceeds the
        /// allowed maximum value.
        RTT::Event<void(std::string)> maximumLoadEvent;

        /// Method to get the maximum measurement value
        RTT::Method<KDL::Wrench(void)> maxMeasurement_mtd;
        /// Method to get the minimum measurement value
        RTT::Method<KDL::Wrench(void)> minMeasurement_mtd;
        /// Method to get the zero measurement value
        RTT::Method<KDL::Wrench(void)> zeroMeasurement_mtd;
        /// Command to choose a different filter
        RTT::Command<bool(double)> chooseFilter_cmd;
        /// Method to set the zero measurement offset
        RTT::Command<bool(KDL::Wrench)> setOffset_cmd;
        /// Method to add an offset to the zero measurement
        RTT::Command<bool(KDL::Wrench)> addOffset_cmd;

        RTT::Property<KDL::Wrench>   offset;
        RTT::Property<unsigned int>  dsp_prop;
        RTT::Property<double>        filter_period_prop;

    private:
        virtual KDL::Wrench maxMeasurement() const;
        virtual KDL::Wrench minMeasurement() const;
        virtual KDL::Wrench zeroMeasurement() const;

        virtual bool chooseFilter(double period);
        virtual bool chooseFilterDone() const;

        virtual bool setOffset(KDL::Wrench in);
        virtual bool addOffset(KDL::Wrench in);
        virtual bool setOffsetDone() const;

        unsigned int  filterToReadFrom;
        unsigned int  dsp;

        KDL::Wrench  writeBuffer;
        s16Forces    write_struct,full_scale;

    };
}//namespace

#endif
