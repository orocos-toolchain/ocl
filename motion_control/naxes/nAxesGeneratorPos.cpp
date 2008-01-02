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

#include "nAxesGeneratorPos.hpp"
#include <ocl/ComponentLoader.hpp>
#include <assert.h>

namespace OCL
{
    using namespace RTT;
    using namespace KDL;
    using namespace std;
    typedef nAxesGeneratorPos MyType;

    nAxesGeneratorPos::nAxesGeneratorPos(string name)
        : TaskContext(name,PreOperational),
          moveTo_cmd( "moveTo",&MyType::moveTo,&MyType::moveFinished, this),
          reset_position_mtd( "resetPosition", &MyType::resetPosition, this),
          p_m_port("nAxesSensorPosition"),
          p_d_port("nAxesDesiredPosition"),
          v_d_port("nAxesDesiredVelocity"),
          v_max_prop("max_vel", "Maximum Velocity in Trajectory"),
          a_max_prop("max_acc", "Maximum Acceleration in Trajectory"),
          num_axes_prop("num_axes","Number of Axes")
    {
        //Creating TaskContext
        
        //Adding properties
        this->properties()->addProperty(&num_axes_prop);
        this->properties()->addProperty(&v_max_prop);
        this->properties()->addProperty(&a_max_prop);
        
        //Adding ports
        this->ports()->addPort(&p_m_port);
        this->ports()->addPort(&p_d_port);
        this->ports()->addPort(&v_d_port);
    
        //Adding Commands
        this->commands()->addCommand( &moveTo_cmd,"Set the position setpoint",
                                      "setpoint", "joint setpoint for all axes",
                                      "time", "minimum time to complete trajectory" );

        //Adding Methods
        this->methods()->addMethod( &reset_position_mtd, "Reset generator position" );  

    }
    
    nAxesGeneratorPos::~nAxesGeneratorPos()
    {}

    bool nAxesGeneratorPos::configureHook()
    {
        num_axes=num_axes_prop.rvalue();
        if(v_max_prop.value().size()!=num_axes){
            Logger::In in(this->getName().data());
            log(Error)<<"Size of "<<v_max_prop.getName()
                      <<" does not match "<<num_axes_prop.getName()
                      <<endlog();
            return false;
        }

        if(a_max_prop.value().size()!=num_axes){
            Logger::In in(this->getName().data());
            log(Error)<<"Size of "<<a_max_prop.getName()
                      <<" does not match "<<num_axes_prop.getName()
                      <<endlog();
            return false;
        }
        
        v_max=v_max_prop.rvalue();
        a_max=a_max_prop.rvalue();
        
        //Resizing all containers to correct size
        p_m.resize(num_axes);
        p_d.resize(num_axes);
        v_d.resize(num_axes);
        motion_profile.resize(num_axes);
        
        //Initialise motion profiles
        for(unsigned int i=0;i<num_axes;i++)
            motion_profile[i].SetMax(v_max[i],a_max[i]);
                    
        //Initialise output ports:
        p_d.assign(num_axes,0);
        p_d_port.Set(p_d);
        v_d.assign(num_axes,0);
        v_d_port.Set(v_d);
        return true;
    }


    bool nAxesGeneratorPos::startHook()
    {
        //check connection and sizes of input-ports
        if(!p_m_port.ready()){
            Logger::In in(this->getName().data());
            log(Error)<<p_m_port.getName()<<" not ready"<<endlog();
            return false;
        }
        if(p_m_port.Get().size()!=num_axes){
            Logger::In in(this->getName().data());
            log(Error)<<"Size of "<<p_m_port.getName()<<": "<<p_m_port.Get().size()<<" != " << num_axes<<endlog();
            return false;
        }

        is_moving = false;
    
        return true;
    }
  
    void nAxesGeneratorPos::updateHook()
    {
        if (is_moving){
            time_passed = TimeService::Instance()->secondsSince(time_begin);
            if ( time_passed > max_duration ){// Profile is ended
                // set end position
                for (unsigned int i=0; i<num_axes; i++){
                    p_d[i] = motion_profile[i].Pos( max_duration );
                    v_d[i] = 0;//_motion_profile[i]->Vel( _max_duration );
                    is_moving = false;
                }
            }else{
                for(unsigned int i=0; i<num_axes; i++){
                    p_d[i] = motion_profile[i].Pos( time_passed );
                    v_d[i] = motion_profile[i].Vel( time_passed );
                }
            }
            p_d_port.Set(p_d);
            v_d_port.Set(v_d);
        }
    }
  
  
    void nAxesGeneratorPos::stopHook()
    {
    }
  
    bool nAxesGeneratorPos::moveTo(const vector<double>& position, double time)
    {
        if(position.size()!=num_axes){
            Logger::In in((this->getName()+moveTo_cmd.getName()).data());
            log(Error)<<"Size of position != "<<num_axes<<endlog();
            return false;
        }
        
        // if previous movement is finished
        if (!is_moving){
            max_duration = 0;
            // get current position/
            p_m = p_m_port.Get();
            for (unsigned int i=0; i<num_axes; i++){
                // Set motion profiles
                motion_profile[i].SetProfileDuration( p_m[i], position[i], time );
                // Find lengthiest trajectory
                max_duration = max( max_duration, motion_profile[i].Duration() );
            }
            // Rescale trajectories to maximal duration
            for(unsigned int i = 0; i < num_axes; i++)
                motion_profile[i].SetProfileDuration( p_m[i], position[i], max_duration );
          
            time_begin = TimeService::Instance()->getTicks();
            time_passed = 0;
          
            is_moving = true;
            return true;
        }
        // still moving
        else{
            Logger::In in((this->getName()+moveTo_cmd.getName()).data());
            log(Error)<<"Still moving, not executing new command."<<endlog();
            return false;
        }
        
    }
  
    bool nAxesGeneratorPos::moveFinished() const
    {
        return (!is_moving);
    }
  
    void nAxesGeneratorPos::resetPosition()
    {
        p_d = p_m_port.Get();
        for(unsigned int i = 0; i < num_axes; i++)
            v_d[i] = 0;
        p_d_port.Set(p_d);
        v_d_port.Set(v_d);
        is_moving = false;
    }
}//namespace

ORO_LIST_COMPONENT_TYPE( OCL::nAxesGeneratorPos )




