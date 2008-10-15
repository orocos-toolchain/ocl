// Copyright (C) 2008 Ruben Smits <ruben.smits@mech.kuleuven.ac.be>
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

#include "nAxesControllerPosVelAcc.hpp"
#include <ocl/ComponentLoader.hpp>
#include <assert.h>

ORO_LIST_COMPONENT_TYPE( OCL::nAxesControllerPosVelAcc )

namespace OCL{

    using namespace RTT;
    using namespace std;

    nAxesControllerPosVelAcc::nAxesControllerPosVelAcc(string name)
        : TaskContext(name,PreOperational),
          reset_all_mtd( "resetAll", &nAxesControllerPosVelAcc::resetAll, this),
          resetAxis_mtd( "resetAxis", &nAxesControllerPosVelAcc::resetAxis, this),
          p_m_port("nAxesSensorPosition"),
          v_m_port("nAxesSensorVelocity"),
          p_d_port("nAxesDesiredPosition"),
          v_d_port("nAxesDesiredVelocity"),
          a_d_port("nAxesDesiredAcceleration"),
          a_out_port("nAxesOutputAcceleration"),
          Kp_prop("Kp", "Position Feedback Gain"),
          //Kv_prop("Kv", "Velocity Feedback Gain"),
          num_axes_prop("num_axes","Number of Axes"),
          use_ad("use_ad","False if no desired acceleration available"),
          use_vd("use_vd","False if no desired acceleration available"),
          use_pd("use_pd","False if no desired acceleration available"),
          avoid_drift("avoid_drift","True if acceleration or velocity should be integrated to avoid drifting"),
          differentiate("differentiate","True if acceleration or velocity should be calculated by differentiation")
    {
        //Creating TaskContext

        //Adding Ports
        this->ports()->addPort(&p_m_port);
        this->ports()->addPort(&v_m_port);
        this->ports()->addPort(&p_d_port);
        this->ports()->addPort(&v_d_port);
        this->ports()->addPort(&a_d_port);
        this->ports()->addPort(&a_out_port);

        //Adding Properties
        this->properties()->addProperty(&num_axes_prop);
        this->properties()->addProperty(&Kp_prop);
        //this->properties()->addProperty(&Kv_prop);
        this->properties()->addProperty(&use_ad);
        this->properties()->addProperty(&use_vd);
        this->properties()->addProperty(&use_pd);
        this->properties()->addProperty(&avoid_drift);
        this->properties()->addProperty(&differentiate);
        
        //Adding Methods
        this->methods()->addMethod( &reset_all_mtd,"reset all axes of the controller");
        this->methods()->addMethod( &resetAxis_mtd,"reset a single axis of the controller",
                                    "axis","axis to reset");

    }


    nAxesControllerPosVelAcc::~nAxesControllerPosVelAcc(){};

    bool nAxesControllerPosVelAcc::configureHook(){
        num_axes=num_axes_prop.rvalue();
        

        //Checking sizes of gain properties
        if(Kp_prop.value().size()!=num_axes){
            Logger::In in(this->getName().data());
            log(Error)<<"Size of "<<Kp_prop.getName()
                      <<" does not match "<<num_axes_prop.getName()
                      <<endlog();
            return false;
        }
/*
        if(Kv_prop.value().size()!=num_axes){
            Logger::In in(this->getName().data());
            log(Error)<<"Size of "<<Kv_prop.getName()
                      <<" does not match "<<num_axes_prop.getName()
                      <<endlog();
            return false;
        }
*/
        //Resizing all containers to correct size
        Kp.resize(num_axes);
        Kp=Kp_prop.rvalue();
        Kv.resize(num_axes);
        //Kv=Kv_prop.rvalue();

        p_m.resize(num_axes);
        p_d.resize(num_axes);
        v_m.resize(num_axes);
        v_d.resize(num_axes);
        v_d_prev.resize(num_axes);
        a_d.resize(num_axes);
        
        is_initialized.resize(num_axes);
        time_begin.resize(num_axes);
        time_difference.resize(num_axes);
        
        //Initialise output ports:
        a_out.assign(num_axes,0);
        a_out_port.Set(a_out);
        return true;

    }

    bool nAxesControllerPosVelAcc::startHook()
    {
        //check connection and sizes of input-ports
        
        if(!p_m_port.ready()){
            Logger::In in(this->getName().data());
            log(Error)<<p_m_port.getName()<<" not ready"<<endlog();
            return false;
        }
        if(!v_m_port.ready()){
            Logger::In in(this->getName().data());
            log(Error)<<v_m_port.getName()<<" not ready"<<endlog();
            return false;
        }

        // reset integrator
        for(unsigned int i=0; i<num_axes; i++)
            is_initialized[i] = false;
        
        //Read property values
        if(Kp_prop.rvalue().size()!=num_axes)//||Kv_prop.rvalue().size()!=num_axes)
            return false;
        
        Kp=Kp_prop.rvalue();
        for(unsigned int i=0;i<num_axes;i++)
            Kv[i]=2*0.7*sqrt(Kp[i]);//damping of 0.7
        
        return true;
    }

    void nAxesControllerPosVelAcc::updateHook()
    {
        // copy Input and Setpoint to local values
        p_m_port.Get(p_m);
        v_m_port.Get(v_m);
        

        // initialize integrator
        for (unsigned int i=0; i<num_axes; i++){
            if (!is_initialized[i]){
                is_initialized[i] = true;
                v_d[i] = v_m[i];
                p_d[i] = p_m[i];
                time_begin[i] = TimeService::Instance()->getTicks();
            }
            time_difference[i] = TimeService::Instance()->secondsSince(time_begin[i]);
            time_begin[i] = TimeService::Instance()->getTicks();
        
            if(avoid_drift){
                //If there should be drift avoidance, calculate the
                //new desired values
                if(!use_vd&&use_ad)
                    v_d[i] += a_d[i] * time_difference[i];
                if(!use_pd)
                    p_d[i] += v_d[i] * time_difference[i];
            }
        }
        
        if(differentiate){
            if(!use_vd&&use_pd)
                p_d_prev = p_d;
            if(!use_ad)
                v_d_prev = v_d;
        }
        
        if(use_ad)
            a_d_port.Get(a_d);
        if(use_vd)
           v_d_port.Get(v_d);
        if(use_pd)
           p_d_port.Get(p_d);

        if(differentiate){
            for(unsigned int i=0;i<num_axes;i++){
                if(!use_vd&&use_pd)
                    v_d[i] = (p_d[i]-p_d_prev[i])/time_difference[i];
                if(!use_ad)
                    a_d[i] = (v_d[i]-v_d_prev[i])/time_difference[i];
            }
        }
        
        if(!use_pd)
            p_d_port.Set(p_d);
        if(!use_vd)
            v_d_port.Set(v_d);
        if(!use_ad)
            a_d_port.Set(a_d);


        // a_out = ad + Kv(vd-vm) + Kp(pd-pm)
        for(unsigned int i=0; i<num_axes; i++){
            a_out[i] = a_d[i] + Kv[i]*(v_d[i]-v_m[i])
                + Kp[i]*(p_d[i]-p_m[i]);
        }
        a_out_port.Set(a_out);
    }

    void nAxesControllerPosVelAcc::stopHook()
    {
        for(unsigned int i=0; i<num_axes; i++){
            a_out[i] = 0.0;
        }
        a_out_port.Set(a_out);
    }

    void nAxesControllerPosVelAcc::resetAll()
    {
        for(unsigned int i=0; i<num_axes; i++)
            is_initialized[i] = false;
    }

    void nAxesControllerPosVelAcc::resetAxis(int axis)
    {
        is_initialized[axis] = false;
    }
}//namespace

