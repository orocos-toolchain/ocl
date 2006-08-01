// Copyright (C) 2006 Wim Meeussen <wim dot neeysseb at mech dot kuleuven dot be>
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
#include <rtt/GenericTaskContext.hpp>
#include <rtt/Ports.hpp>
#include <rtt/Event.hpp>
#include <rtt/Properties.hpp>

namespace Orocos
{
    
  class Demotool : public RTT::GenericTaskContext
  {
  public:
    /**
     * Construct an interface to the Demotool
     */
    Demotool(std::string name, std::string propertyfilename="cpf/Demotool.cpf");
    virtual~Demotool();
    
    virtual bool startup();
    virtual void update();
    virtual void shutdown();
    
  private:
    RTT::Property<std::vector<double> > _pos_leds_demotool;
    RTT::Property<double> _mass_demotool;
    RTT::Property<KDL::Vector> _center_gravity_demotool;
    RTT::Property<KDL::Frame> _demotool_obj;
    RTT::Property<KDL::Frame> _demotool_fs;

    RTT::WriteDataPort<KDL::Frame> _frame_camera_object;
    RTT::WriteDataPort<unsigned int> _num_visible_leds;
    RTT::WriteDataPort<KDL::Wrench> _wrench_object_object;

    std::string _propertyfile;
    
    
   }; // class
} // namespace

#endif
