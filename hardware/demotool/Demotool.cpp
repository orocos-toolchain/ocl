// Copyright (C) 2006 Ruben Smits <ruben dot smits at mech dot kuleuven dot be>
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

#include "Demotool.hpp"


namespace Orocos
{
  using namespace RTT;
  using namespace std;
  using namespace KDL;
  
  Demotool::Demotool(string name, string propertyfile):
    GenericTaskContext(name),
    _pos_leds_demotool("pos_leds_demotool","XYZ positions of all LED markers, relative to demtool frame"),
    _mass_demotool("mass_demotool","mass of objects attached to force censor of demotool"),
    _center_gravity_demotool("center_gravity_demotool","center of gravity of mass attached to demotool"),
    _demotool_obj("demotool_obj","frame from demotool to object"),
    _demotool_fs("demotool_fs","frame from demotool to force sensor"),
    _frame_camera_object("frame_camera_object"),
    _num_visible_leds("num_visible_leds"),
    _wrench_object_object("wrench_object_object"),
    _propertyfile(propertyfile)
  {
    Logger::log()<<Logger::Debug<<this->getName()<<": adding Properties"<<Logger::endl;
    properties()->addProperty(&_pos_leds_demotool);
    properties()->addProperty(&_mass_demotool);
    properties()->addProperty(&_center_gravity_demotool);
    properties()->addProperty(&_demotool_obj);
    properties()->addProperty(&_demotool_fs);

    Logger::log()<<Logger::Debug<<this->getName()<<": adding Ports"<<Logger::endl;
    ports()->addPort(&_frame_camera_object);
    ports()->addPort(&_num_visible_leds);
    ports()->addPort(&_wrench_object_object);
    
    Logger::log()<<Logger::Debug<<this->getName()<<": adding Events"<<Logger::endl;
    //events()->addEvent("distanceOutOfRange",&_distanceOutOfRange);
    
    if(!readProperties(_propertyfile))
      log(Error)<<"Reading properties failed."<<endlog();
    
    Logger::log()<<Logger::Debug<<this->getName()<<": constructed."<<Logger::endl;
  }
  
  Demotool::~Demotool()
  {
  }
  
  bool Demotool::startup()    
  {
    return true;
  }
    
  void Demotool::update()
  {
    _num_visible_leds.Set(6);
  }
  
  void Demotool::shutdown()
  {
    writeProperties(_propertyfile);
  }
}//namespace

