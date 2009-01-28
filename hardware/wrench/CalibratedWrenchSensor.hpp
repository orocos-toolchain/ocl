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

#ifndef _CALIBRATED_WRENCH_SENSOR_HPP_
#define _CALIBRATED_WRENCH_SENSOR_HPP_

#include <rtt/TaskContext.hpp>
#include <rtt/Ports.hpp>
#include <rtt/Property.hpp>

#include <ocl/OCL.hpp>
#include <kdl/frames.hpp>

namespace OCL{

class CalibratedWrenchSensor : public RTT::TaskContext
{
public:
    CalibratedWrenchSensor(const std::string& name);
    virtual ~CalibratedWrenchSensor(){};
    
private:
    virtual bool configureHook();
    virtual bool startHook();
    virtual void updateHook();
    virtual void stopHook();
    virtual void cleanupHook(){};
    
    RTT::ReadDataPort<KDL::Wrench> wrench_in;
    RTT::ReadDataPort<KDL::Frame> mount_position;
    
    RTT::WriteDataPort<KDL::Wrench> wrench_out;

    RTT::Property<KDL::Frame> WF_prop;
    RTT::Property<KDL::Frame> CF_prop;
    RTT::Property<KDL::Vector> cog_prop;
    RTT::Property<double> mass_prop;

    KDL::Vector cog;
    KDL::Frame F_m_comp,F_m_ws;
    double mass;
};
}

#endif
