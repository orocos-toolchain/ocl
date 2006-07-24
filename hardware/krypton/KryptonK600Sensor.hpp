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


#ifndef __KRYTON_K600_SENSOR__
#define __KRYTON_K600_SENSOR__

#include <pkgconf/system.h> 

#include <rtt/RTT.hpp>
#include <geometry/GeometryToolkit.hpp>
#include <rtt/GenericTaskContext.hpp>
#include <rtt/Ports.hpp>

#if defined (OROPKG_OS_LXRT)
#include <rtai_mbx.h>
#define MAX_NUM_LEDS 20 // Maximum number of leds
#define NUM_COORD_PER_LED 3 // Number of coordinates per led 
#define MAX_NUM_COORD (MAX_NUM_LEDS*NUM_COORD_PER_LED)
#define MAX_MESSAGE_LENGTH (15 + MAX_NUM_COORD * 8)
#endif

namespace Orocos
{
  class KryptonK600Sensor : public RTT::GenericTaskContext
  {
    
  public:
    KryptonK600Sensor(std::string name, int num_leds);
    virtual ~KryptonK600Sensor();
    
    virtual bool startup();
    virtual void update();
    virtual void shutdown();
    
  private:

    int _num_leds;

#if defined (OROPKG_OS_LXRT)
    unsigned short _length_msg, _type_msg, _nr_msg, _nr_answer_msg, _type_body_msg, _cycle_nr, _nr_markers;
    unsigned char _msg_valid;

    SEM * udp_message_arrived;
    MBX * udp_message;

    char* _msg;

    bool interprete_K600_Msg();
#endif

    std::vector<ORO_Geometry::Vector> _ledPositions_local;
    RTT::WriteDataPort<std::vector<ORO_Geometry::Vector> > _ledPositions;
  };
};//namespace

#endif
  
