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

#include "CalibratedWrenchSensor.hpp"

#define G 9.811
namespace OCL{
    using namespace RTT;
    using namespace KDL;

    CalibratedWrenchSensor::CalibratedWrenchSensor(const std::string& name):
        RTT::TaskContext(name,PreOperational),
        wrench_in("WrenchData"),
        mount_position("MountPosition"),
        wrench_out("CalibratedWrench",Wrench::Zero()),
        WF_prop("SensorFrame","Pose between the Mount Position and the WrenchSensor Frame",Frame::Identity()),
        CF_prop("ComplianceFrame","Pose between the Mount Position and the Frame in which to express the wrench",Frame::Identity()),
        cog_prop("cog","Center of Gravity of attached tool",Vector::Zero()),
        mass_prop("mass","Mass of attached tool",0.0)
    {
        ports()->addPort(&mount_position);
        ports()->addPort(&wrench_in);
        ports()->addPort(&wrench_out);
        
        properties()->addProperty(&WF_prop);
        properties()->addProperty(&CF_prop);
        properties()->addProperty(&cog_prop);
        properties()->addProperty(&mass_prop);

    }
    
    bool CalibratedWrenchSensor::configureHook(){
        mass=mass_prop.rvalue();
        cog=cog_prop.rvalue();
        F_m_comp=CF_prop.rvalue();
        F_m_ws=WF_prop.rvalue();

        return true;
    }
    
    bool CalibratedWrenchSensor::startHook(){
        return true;
    }

    void CalibratedWrenchSensor::updateHook(){
        //Get input:
        Wrench measurement(wrench_in.Get());  
        Frame F_b_m(mount_position.Get());
        

        //Calculate the part due to gravity:
        Wrench gravity;
        //gravity force from the base frame into the mount frame into the wrenchsensor frame
        gravity.force  = F_m_ws.M.Inverse(F_b_m.M.Inverse(Vector(0.0, 0.0, -mass*G)));
        gravity.torque = cog * gravity.force;
        //measurement from wrenchsensor to mount frame to complianceframe
        wrench_out.Set(F_m_comp.Inverse(F_m_ws * (measurement - gravity)));

    }
    void CalibratedWrenchSensor::stopHook(){
        wrench_out.Set(Wrench::Zero());
    }
    
}
