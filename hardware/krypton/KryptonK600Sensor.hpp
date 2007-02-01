// Copyright (C) 2006 Klaas Gadeyne <klaas dot gadeyne at mech dot kuleuven dot be>
//                    Ruben Smits <ruben dot smits at mech dot kuleuven dot be>
//                    Wim Meeussen <wim dot meeussen at mech dot kuleuven dot be>
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

#include <rtt/RTT.hpp> 
#include <kdl/frames.hpp>
#include <rtt/RTT.hpp>
#include <rtt/TaskContext.hpp>
#include <rtt/NonPeriodicActivity.hpp>
#include <rtt/Ports.hpp>

#if defined (OROPKG_OS_LXRT)
#include <rtai_mbx.h>
#define MAX_NUM_LEDS 20 // Maximum number of leds
#define NUM_COORD_PER_LED 3 // Number of coordinates per led 
#define MAX_NUM_COORD (MAX_NUM_LEDS*NUM_COORD_PER_LED)
#define MAX_MESSAGE_LENGTH (15 + MAX_NUM_COORD * 8)
#endif

#include <ocl/OCL.hpp>

namespace OCL
{
  class KryptonK600Sensor 
    :public RTT::TaskContext
#if defined (OROPKG_OS_LXRT)
    ,public RTT::NonPeriodicActivity
#endif
  {
    
  public:
    KryptonK600Sensor(std::string name, unsigned int num_leds, unsigned int priority=0);
    virtual ~KryptonK600Sensor();

  private:
    virtual bool startup(){return true;}
    virtual void update(){};  // taskcontext
    virtual void shutdown(){};

#if defined (OROPKG_OS_LXRT)
    unsigned short _length_msg, _type_msg, _nr_msg, _nr_answer_msg, _type_body_msg, _cycle_nr, _nr_markers;
    unsigned char _msg_valid;

    SEM * udp_message_arrived;
    MBX * udp_message;

    bool _keep_running;

    bool interprete_K600_Msg(char* msg);

    virtual void loop();  // nonperiodicactivity
#endif

    unsigned int _num_leds;

    std::vector<KDL::Vector> _ledPositions_local;
    RTT::WriteDataPort<std::vector<KDL::Vector> > _ledPositions;
  };
};//namespace

#endif
  
