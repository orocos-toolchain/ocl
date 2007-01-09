// Copyright (C) 2006 Wim Meeussen <wim dot meeussen at mech dot kuleuven dot be>
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

#ifndef __DEMOTOOL_HARDWARE__
#define __DEMOTOOL_HARDWARE__

#include <rtt/RTT.hpp>
#include <rtt/TaskContext.hpp>
#include <rtt/Ports.hpp>
#include <rtt/Event.hpp>
#include <rtt/Properties.hpp>
#include <rtt/Method.hpp>
#include <rtt/Command.hpp>
#include <kdl/frames.hpp>

#include <ocl/OCL.hpp>

namespace OCL
{
    
  /**
   * This class implements a TaskContext to use with the demonstration
   * tool in the RoboticLab, PMA, dept. Mechanical Engineering,
   * KULEUVEN. Since the hardware part is very specific for our setup,
   * other people cannot use this class.
   */
  class Demotool : public RTT::TaskContext
  {
  public:
    /** 
     * The contructor of the class.
     * 
     * @param name Name of the TaskContext
     * @param propertyfilename name of the propertyfile to
     * configure the component with, default: cpf/Demotool.cpf
     */
    Demotool(std::string name, std::string propertyfilename="cpf/Demotool.cpf");
    virtual~Demotool();
    virtual bool startup();
    virtual void update();
    virtual void shutdown();

  protected:
    // property
    RTT::Property<std::vector<double> > _pos_leds_demotool;
    RTT::Property<double>               _mass_demotool;
    RTT::Property<KDL::Vector>          _center_gravity_demotool, _gravity_dir_world;
    RTT::Property<KDL::Frame>           _Frame_demotool_manip, _Frame_demotool_fs, _Frame_world_camera;

    // read ports
    RTT::ReadDataPort<KDL::Wrench>               _Wrench_fs_fs_port;
    RTT::ReadDataPort<std::vector<KDL::Vector> > _Vector_led_camera_port;

    // write ports
    RTT::WriteDataPort<KDL::Wrench>  _Wrench_world_world_port, _Wrench_manip_manip_port;
    RTT::WriteDataPort<KDL::Twist>   _Twist_world_world_port, _Twist_world_manip_port;
    RTT::WriteDataPort<KDL::Frame>   _Frame_world_manip_port;
    RTT::WriteDataPort<unsigned int> _num_visible_leds_port;

    // commands
    RTT::Command<bool(KDL::Wrench)>  _add_offset;

    // methods
    RTT::Method<void(void)>          _calibrate_world_to_manip, _calibrate_wrench_sensor;

  private:
    void calibrateWorldToManip();
    void calibrateWrenchSensor();
    
    std::string _propertyfile;
    std::vector<KDL::Vector> _Vector_led_demotool, _Vector_led_camera;
    
    KDL::Twist   _Twist_world_manip;
    KDL::Wrench  _Wrench_fs_fs, _Wrench_world_world, _Wrench_gravity_world_world;
    KDL::Frame   _Frame_world_demotool, _Frame_camera_demotool, _Frame_world_manip, _Frame_world_manip_old, _Frame_world_fs;
    unsigned int _num_visible_leds, _num_leds;
    std::vector<bool> _visible_leds;
    bool _is_initialized;
    double _period;
    RTT::TimeService::ticks _time_begin;
    

   }; // class
} // namespace

#endif
