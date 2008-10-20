// $Id: nAxisGeneratorCartesianPos.hpp,v 1.1.1.1 2003/12/02 20:32:06 kgadeyne Exp $
// Copyright (C) 2003 Klaas Gadeyne <klaas.gadeyne@mech.kuleuven.ac.be>
//                    Wim Meeussen  <wim.meeussen@mech.kuleuven.ac.be>
// Copyright (C) 2006 Ruben Smits <ruben.smits@mech.kuleuven.ac.be>
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

#include "nAxesControllerVel.hpp"
#include <ocl/ComponentLoader.hpp>
#include <assert.h>

namespace OCL{

    using namespace RTT;
    using namespace std;
    typedef nAxesControllerVel MyType;

    nAxesControllerVel::nAxesControllerVel(string name)
        : TaskContext(name,PreOperational),
          reset_all_mtd( "resetAll", &MyType::resetAll, this),
          resetAxis_mtd( "resetAxis", &MyType::resetAxis, this),
          p_m_port("nAxesSensorPosition"),
          v_d_port("nAxesDesiredVelocity"),
          v_out_port("nAxesOutputVelocity"),
          gain_prop("K", "Proportional Gain"),
          num_axes_prop("num_axes","Number of Axes")
    {
        //Creating TaskContext

        //Adding Ports
        this->ports()->addPort(&p_m_port);
        this->ports()->addPort(&v_d_port);
        this->ports()->addPort(&v_out_port);

        //Adding Properties
        this->properties()->addProperty(&num_axes_prop);
        this->properties()->addProperty(&gain_prop);

        //Adding Methods
        this->methods()->addMethod( &reset_all_mtd,"reset all axes of the controller");
        this->methods()->addMethod( &resetAxis_mtd,"reset a single axis of the controller",
                                    "axis","axis to reset");

    }


    nAxesControllerVel::~nAxesControllerVel(){};

    bool nAxesControllerVel::configureHook(){
        num_axes=num_axes_prop.rvalue();
        if(gain_prop.value().size()!=num_axes){
            Logger::In in(this->getName().data());
            log(Error)<<"Size of "<<gain_prop.getName()
                      <<" does not match "<<num_axes_prop.getName()
                      <<endlog();
            return false;
        }

        gain.resize(num_axes);
        gain=gain_prop.rvalue();

        //Resizing all containers to correct size
        p_m.resize(num_axes);
        p_d.resize(num_axes);
        v_d.resize(num_axes);
        is_initialized.resize(num_axes);
        time_begin.resize(num_axes);

        //Initialise output ports:
        v_out.assign(num_axes,0);
        v_out_port.Set(v_out);
        return true;

    }

    bool nAxesControllerVel::startHook()
    {
                //check connection and sizes of input-ports
        if(!p_m_port.ready()){
            Logger::In in(this->getName().data());
            log(Error)<<p_m_port.getName()<<" not ready"<<endlog();
            return false;
        }
        if(!v_d_port.ready()){
            Logger::In in(this->getName().data());
            log(Error)<<v_d_port.getName()<<" not ready"<<endlog();
            return false;
        }
        if(p_m_port.Get().size()!=num_axes){
            Logger::In in(this->getName().data());
            log(Error)<<"Size of "<<p_m_port.getName()<<": "<<p_m_port.Get().size()<<" != " << num_axes<<endlog();
            return false;
        }
        if(v_d_port.Get().size()!=num_axes){
            Logger::In in(this->getName().data());
            log(Error)<<"Size of "<<v_d_port.getName()<<": "<<v_d_port.Get().size()<<" != " << num_axes<<endlog();
            return false;
        }

        // reset integrator
        for(unsigned int i=0; i<num_axes; i++)
            is_initialized[i] = false;

        return true;
    }

    void nAxesControllerVel::updateHook()
    {
        // copy Input and Setpoint to local values
        p_m = p_m_port.Get();
        v_d = v_d_port.Get();

        // initialize integrator
        for (unsigned int i=0; i<num_axes; i++){
            if (!is_initialized[i]){
                is_initialized[i] = true;
                p_d[i] = p_m[i];
                time_begin[i] = TimeService::Instance()->getTicks();
            }
        }

        // position feedback on integrated velocity
        for(unsigned int i=0; i<num_axes; i++){
            double time_difference = TimeService::Instance()->secondsSince(time_begin[i]);
            p_d[i] += v_out[i] * time_difference;
            v_out[i] = (gain[i] * (p_d[i] - p_m[i])) + v_d[i];
            time_begin[i] = TimeService::Instance()->getTicks();
        }
        v_out_port.Set(v_out);
    }

    void nAxesControllerVel::stopHook()
    {
        for(unsigned int i=0; i<num_axes; i++){
            v_out[i] = 0.0;
        }
        v_out_port.Set(v_out);
    }

    void nAxesControllerVel::resetAll()
    {
        for(unsigned int i=0; i<num_axes; i++)
            is_initialized[i] = false;
    }

    void nAxesControllerVel::resetAxis(int axis)
    {
        is_initialized[axis] = false;
    }
}//namespace
ORO_LIST_COMPONENT_TYPE( OCL::nAxesControllerVel )

