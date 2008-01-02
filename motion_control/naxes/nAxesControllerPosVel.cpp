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


#include "nAxesControllerPosVel.hpp"
#include <rtt/Logger.hpp>
#include <ocl/ComponentLoader.hpp>

#include <assert.h>

namespace OCL
{
    using namespace RTT;
    using namespace std;
    
    nAxesControllerPosVel::nAxesControllerPosVel(string name)
        : TaskContext(name,PreOperational),
          p_meas_port("nAxesSensorPosition"),
          p_desi_port("nAxesDesiredPosition"),
          v_desi_port("nAxesDesiredVelocity"),
          v_out_port("nAxesOutputVelocity"),
          gain_prop("K", "Proportional Gain"),
          num_axes_prop("num_axes","Number of Axes")
    {
        //Creating TaskContext
        
        //Adding ports
        this->ports()->addPort(&p_meas_port);
        this->ports()->addPort(&p_desi_port);
        this->ports()->addPort(&v_desi_port);
        this->ports()->addPort(&v_out_port);
        
        //Adding properties
        this->properties()->addProperty(&num_axes_prop);
        this->properties()->addProperty(&gain_prop);
        
    }
    
    nAxesControllerPosVel::~nAxesControllerPosVel(){};
    
    bool nAxesControllerPosVel::configureHook(){
        num_axes=num_axes_prop.rvalue();
        if(gain_prop.value().size()!=num_axes){
            Logger::In in(this->getName().data());
            log(Error)<<"Size of "<<gain_prop.getName()
                      <<" does not match "<<num_axes_prop.getName()
                      <<endlog();
            return false;
        }
        
        gain=gain_prop.rvalue();
        
        //Resizing all containers to correct size
        p_meas.resize(num_axes);
        p_desi.resize(num_axes);
        v_desi.resize(num_axes);
                
        //Initialise output ports:
        v_out.assign(num_axes,0);
        v_out_port.Set(v_out);
        return true;

    }
    

    bool nAxesControllerPosVel::startHook(){
        //check connection and sizes of input-ports
        if(!p_meas_port.ready()){
            Logger::In in(this->getName().data());
            log(Error)<<p_meas_port.getName()<<" not ready"<<endlog();
            return false;
        }
        if(!p_desi_port.ready()){
            Logger::In in(this->getName().data());
            log(Error)<<p_desi_port.getName()<<" not ready"<<endlog();
            return false;
        }
        if(!v_desi_port.ready()){
            Logger::In in(this->getName().data());
            log(Error)<<v_desi_port.getName()<<" not ready"<<endlog();
            return false;
        }
        if(p_meas_port.Get().size()!=num_axes){
            Logger::In in(this->getName().data());
            log(Error)<<"Size of "<<p_meas_port.getName()<<": "<<p_meas_port.Get().size()<<" != " << num_axes<<endlog();
            return false;
        }
        if(p_desi_port.Get().size()!=num_axes){
            Logger::In in(this->getName().data());
            log(Error)<<"Size of "<<p_desi_port.getName()<<": "<<p_desi_port.Get().size()<<" != " << num_axes<<endlog();
            return false;
        }        
        if(v_desi_port.Get().size()!=num_axes){
            Logger::In in(this->getName().data());
            log(Error)<<"Size of "<<v_desi_port.getName()<<": "<<v_desi_port.Get().size()<<" != " << num_axes<<endlog();
            return false;
        }        
   
        return true;
    }
    
    void nAxesControllerPosVel::updateHook(){
        // copy Input and Setpoint to local values
        p_meas = p_meas_port.Get();
        p_desi = p_desi_port.Get();
        v_desi = v_desi_port.Get();
        
        for(unsigned int i=0; i<num_axes; i++)
            v_out[i] = (gain[i] * (p_desi[i] - p_meas[i])) + v_desi[i];
        
        v_out_port.Set(v_out);
    }
  
    void nAxesControllerPosVel::stopHook(){
        for(unsigned int i=0; i<num_axes; i++){
            v_out[i] = 0.0;
        }
        v_out_port.Set(v_out);
    }
}//namespace
ORO_LIST_COMPONENT_TYPE( OCL::nAxesControllerPosVel )
