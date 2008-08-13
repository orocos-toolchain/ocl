// $Id: nAxisGeneratorCartesianVel.hpp,v 1.1.1.1 2003/12/02 20:32:06 kgadeyne Exp $
// Copyright (C) 2003 Klaas Gadeyne <klaas.gadeyne@mech.kuleuven.ac.be>
//                    Wim Meeussen  <wim.meeussen@mech.kuleuven.ac.be>
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

#include "nAxesGeneratorVel.hpp"
#include <ocl/ComponentLoader.hpp>
#include <assert.h>

namespace OCL
{
    using namespace RTT;
    using namespace KDL;
    using namespace std;
    typedef nAxesGeneratorVel MyType;

    nAxesGeneratorVel::nAxesGeneratorVel(string name)
        : TaskContext(name,PreOperational),
          applyVelocity_cmd( "applyVelocity", &MyType::applyVelocity,
                          &MyType::velocityFinished, this),
          applyVelocities_cmd( "applyVelocities", &MyType::applyVelocities,
                            &MyType::velocitiesFinished, this),
          gotoVelocity_cmd( "gotoVelocity", &MyType::gotoVelocity,
                         &MyType::velocityFinished, this),
          gotoVelocities_cmd( "gotoVelocities", &MyType::gotoVelocities,
                           &MyType::velocitiesFinished, this),
          setInitVelocity_mtd( "setInitVelocity", &MyType::setInitVelocity, this),
          setInitVelocities_mtd( "setInitVelocities", &MyType::setInitVelocities, this),
          v_d_port("nAxesDesiredVelocity"),
          a_max_prop("max_acc", "Maximum Acceleration in Trajectory"),
          j_max_prop("max_jerk", "Maximum Jerk in Trajectory"),
          num_axes_prop("num_axes","Number of Axes")
    {
        //Creating TaskContext

        //Adding Properties
        this->properties()->addProperty(&num_axes_prop);
        this->properties()->addProperty(&a_max_prop);
        this->properties()->addProperty(&j_max_prop);

        //Adding Ports
        this->ports()->addPort(&v_d_port);

        //Creating commands
        this->commands()->addCommand( &applyVelocities_cmd,"Set the velocity",
                                      "velocity", "joint velocity for all axes",
                                      "duration", "duration of movement" );
        this->commands()->addCommand( &applyVelocity_cmd,"Set the velocity for one axis",
                                      "axis", "selected axis",
                                      "velocity", "joint velocity for axis",
                                      "duration", "duration of movement" );
        this->commands()->addCommand( &gotoVelocities_cmd,"Set the velocities",
                                      "velocities", "joint velocities for all axes",
                                      "duration", "duration of movement" );
        this->commands()->addCommand( &gotoVelocity_cmd,"Set the velocity for one axis",
                                      "axis", "selected axis",
                                      "velocity", "joint velocity for axis",
                                      "duration", "duration of movement" );

        //Creating Methods

        this->methods()->addMethod( &setInitVelocity_mtd,"set initial velocity",
                                    "axis", "axis where to set velocity",
                                    "velocity", "velocity to set" );
        this->methods()->addMethod( &setInitVelocities_mtd,"set initial velocity",
                                    "velocities", "velocities to set" );

    }

    nAxesGeneratorVel::~nAxesGeneratorVel()
    {}

    bool nAxesGeneratorVel::configureHook()
    {
        num_axes=num_axes_prop.rvalue();
        if(a_max_prop.value().size()!=num_axes){
            Logger::In in(this->getName().data());
            log(Error)<<"Size of "<<a_max_prop.getName()
                      <<" does not match "<<num_axes_prop.getName()
                      <<endlog();
            return false;
        }

        if(j_max_prop.value().size()!=num_axes){
            Logger::In in(this->getName().data());
            log(Error)<<"Size of "<<j_max_prop.getName()
                      <<" does not match "<<num_axes_prop.getName()
                      <<endlog();
            return false;
        }

        a_max=a_max_prop.rvalue();
        j_max=j_max_prop.rvalue();

        //Resizing all containers to correct size
        duration_desired.resize(num_axes);
        duration_trajectory.resize(num_axes);
        maintain_velocity.resize(num_axes);
        time_begin.resize(num_axes);
        time_passed.resize(num_axes);
        vel_profile.resize(num_axes);

        //Initialise motion profiles
        for(unsigned int i=0;i<num_axes;i++)
            vel_profile[i].SetMax(a_max[i],j_max[i]);

        //Initialise output ports:
        v_d.assign(num_axes,0);
        v_d_port.Set(v_d);
        return true;
    }


    bool nAxesGeneratorVel::startHook()
    {
        for (unsigned int i=0; i<num_axes; i++){
            maintain_velocity[i] = false;
        }

        // generate initial trajectory to maintain current velocity
        for (unsigned int i=0; i<num_axes; i++)
            applyVelocity(i, v_d[i], 0.0);

        return true;
    }


    void nAxesGeneratorVel::updateHook()
    {
        for (unsigned int i = 0 ; i < num_axes ; i++)
            time_passed[i] = TimeService::Instance()->secondsSince(time_begin[i]);

        for (unsigned int i = 0 ; i < num_axes ; i++){
            // still moving
            if (maintain_velocity[i] || time_passed[i] <= duration_desired[i] || duration_desired[i] == 0)
                v_d[i] = vel_profile[i].Pos( min(time_passed[i], duration_trajectory[i]) );

            // stop moving if time is up
            else
                applyVelocity(i, 0.0, 0.0);
        }

        v_d_port.Set(v_d);
    }

    void nAxesGeneratorVel::stopHook()
    {
    }


    bool nAxesGeneratorVel::setInitVelocity(const unsigned int axis, const double velocity)
    {
        if (axis >= num_axes){
            Logger::In in((this->getName()+setInitVelocity_mtd.getName()).data());
            log(Error)<<"parameter axis "<<axis<<" to big."<<endlog();
            return false;
        }else{
            v_d[axis] = velocity;
            applyVelocity(axis, v_d[axis], 0.0);
            return true;
        }
    }


    bool nAxesGeneratorVel::setInitVelocities(const vector<double>& velocity)
    {
        if(velocity.size()!=num_axes){
            Logger::In in((this->getName()+setInitVelocities_mtd.getName()).data());
            log(Error)<<"Size of parameter velocity != "<<num_axes<<endlog();
            return false;
        }
        for (unsigned int i=0; i<num_axes; i++)
            setInitVelocity(i, velocity[i]);
        return true;
    }

    bool nAxesGeneratorVel::gotoVelocity(const unsigned int axis, const double velocity, double duration)
    {
        if (axis >= num_axes){
            Logger::In in((this->getName()+gotoVelocity_cmd.getName()).data());
            log(Error)<<"parameter axis "<<axis<<" to big."<<endlog();
            return false;
        }
        if (duration < 0){
            Logger::In in((this->getName()+gotoVelocity_cmd.getName()).data());
            log(Error)<<"parameter duration < 0"<<endlog();
            return false;
        }

        time_begin[axis]        = TimeService::Instance()->getTicks();
        time_passed[axis]       = 0;
        maintain_velocity[axis] = true;

        // calculate velocity profile
        vel_profile[axis].SetProfileDuration(v_d[axis], velocity, duration);

        // get duration of acceleration
        duration_trajectory[axis] = vel_profile[axis].Duration();
        duration_desired[axis] = duration_trajectory[axis];

        return true;
    }


    bool nAxesGeneratorVel::gotoVelocities(const vector<double>& velocity, double duration)
    {
        if (duration < 0){
            Logger::In in((this->getName()+gotoVelocities_cmd.getName()).data());
            log(Error)<<"parameter duration < 0"<<endlog();
            return false;
        }
        if(velocity.size()!=num_axes){
            Logger::In in((this->getName()+gotoVelocities_cmd.getName()).data());
            log(Error)<<"Size of parameter velocity != "<<num_axes<<endlog();
            return false;
        }

        for (unsigned int i=0; i<num_axes; i++)
            gotoVelocity(i, velocity[i], duration);

        return true;
    }

    bool nAxesGeneratorVel::applyVelocity(const unsigned int axis, const double velocity, double duration)
    {
        if (axis >= num_axes){
            Logger::In in((this->getName()+applyVelocity_cmd.getName()).data());
            log(Error)<<"parameter axis "<<axis<<" to big."<<endlog();
            return false;
        }
        if (duration < 0){
            Logger::In in((this->getName()+applyVelocity_cmd.getName()).data());
            log(Error)<<"parameter duration < 0"<<endlog();
            return false;
        }

        time_begin[axis]        = TimeService::Instance()->getTicks();
        time_passed[axis]       = 0;
        duration_desired[axis]  = duration;
        maintain_velocity[axis] = false;

        // calculate velocity profile
        vel_profile[axis].SetProfile(v_d[axis], velocity);

        // get duration of acceleration
        duration_trajectory[axis] = vel_profile[axis].Duration();

        return true;

    }


    bool nAxesGeneratorVel::applyVelocities(const vector<double>& velocity, double duration)
    {
        if (duration < 0){
            Logger::In in((this->getName()+applyVelocities_cmd.getName()).data());
            log(Error)<<"parameter duration < 0"<<endlog();
            return false;
        }
        if(velocity.size()!=num_axes){
            Logger::In in((this->getName()+applyVelocities_cmd.getName()).data());
            log(Error)<<"Size of parameter velocity != "<<num_axes<<endlog();
            return false;
        }

        for (unsigned int i=0; i<num_axes; i++)
            applyVelocity(i, velocity[i], duration);

        return true;
    }

    bool nAxesGeneratorVel::velocityFinished(const unsigned int axis) const
    {
        return (time_passed[axis] > duration_desired[axis] || duration_desired[axis] == 0);
    }


    bool nAxesGeneratorVel::velocitiesFinished() const
    {
        bool returnValue=true;
        for (unsigned int i = 0 ; i < num_axes ; i++)
            if (! velocityFinished(i))
                returnValue = false;

        return returnValue;
    }
}//namespace

ORO_LIST_COMPONENT_TYPE( OCL::nAxesGeneratorVel )

