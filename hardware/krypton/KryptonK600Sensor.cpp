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

#include "KryptonK600Sensor.hpp"
#include <corelib/Logger.hpp>

namespace Orocos
{
  using namespace RTT;
  using namespace ORO_Geometry;
  using namespace std;
  
  KryptonK600Sensor::KryptonK600Sensor(string name,int num_leds)
    : GenericTaskContext(name),
      _num_leds(num_leds),
      _ledPositions_local(num_leds),
      _ledPositions("LedPositions")
  {
    ports()->addPort(&_ledPositions);
#if defined (OROPKG_OS_LXRT)
    _msg = new char[MAX_MESSAGE_LENGTH];
#endif
  }

  KryptonK600Sensor::~KryptonK600Sensor()
  {
  }
  


  bool KryptonK600Sensor::startup()
  {
#if defined (OROPKG_OS_LXRT)
    // If kernel Module is not loaded yet, Print error message and
    // wait for one second
    if (! ((udp_message_arrived = (SEM *) rt_get_adr(nam2num("KEDSEM"))) && 
	   (udp_message = (MBX *) rt_get_adr(nam2num("KEDMBX")))	      )
	)
      {
	Logger::log() << Logger::Info << "K600PositionInterface: Can't find sem/mbx" << Logger::endl;
	return false;
      }
#endif
    for( unsigned int i =0; i<_num_leds;i++)
      SetToZero(_ledPositions_local[i]);
    
    return true;
  }
  
  
  void KryptonK600Sensor::update()
  {
#if defined (OROPKG_OS_LXRT)

    Logger::log() << Logger::Debug << "K600PositionInterface: update" << Logger::endl;

    // Wait until kernel Module posts semaphore
    rt_sem_wait(udp_message_arrived);

    unsigned int ret;
    if ((ret = rt_mbx_receive_if(udp_message,(void *) &_msg, MAX_MESSAGE_LENGTH)) < 0 )
      Logger::log() << Logger::Info << "K600PositionInterface: Error rcv message from mbx" << Logger::endl;
    else
      {
	// Interprete Message
	if ( interprete_K600_Msg())
	  {
	    Logger::log() << Logger::Debug << "K600PositionInterface: Received message from mbx" << Logger::endl;
	    // Copy Data into write buffer
	    _ledPositions.Set(_ledPositions_local);
	  }
	else Logger::log() << Logger::Info << "K600PositionInterface: Bad message, or something went wrong in decoding" << Logger::endl;
      }
#endif
    _ledPositions.Set(_ledPositions_local);
  }

  void KryptonK600Sensor::shutdown()
  {
    Logger::log() << Logger::Info << "KryptonK600Sensor: shutdown" << Logger::endl;
#if defined (OROPKG_OS_LXRT)
    // In update, we might be waiting for the sem, so first
    // post it...
    SEM * udp_message_arrived;
    udp_message_arrived = (SEM *) rt_get_adr(nam2num("KEDSEM"));
    rt_sem_signal (udp_message_arrived);
#endif
  }
  
#if defined (OROPKG_OS_LXRT)
  bool KryptonK600Sensor::interprete_K600_Msg()
  {
    unsigned short i;
    unsigned int index = 15;

    _length_msg    = (unsigned short)_msg[0];
    _type_msg      = (unsigned short)_msg[2];
    _nr_msg        = (unsigned short)_msg[4];
    _nr_answer_msg = (unsigned short)_msg[6];

    _msg_valid     = _msg[8];

    _type_body_msg = (unsigned short)_msg[9];
    _cycle_nr      = (unsigned short)_msg[11];
    _nr_markers    = (unsigned short)_msg[13];
    
    if (_nr_markers < MAX_NUM_LEDS && _nr_markers!=_num_leds)
      {
	for (i = 0; i < _num_leds ; i++)
	  {
	    _ledPositions_local[i].x((double)_msg[index]);
	    index += 8; // A double is 8 bytes (chars)
	    _ledPositions_local[i].y((double)_msg[index]);
	    index += 8; // A double is 8 bytes (chars)
	    _ledPositions_local[i].z((double)_msg[index]);
	    index += 8; // A double is 8 bytes (chars)
	  }
	return true;
      }
    else 
      {
	Logger::log() << Logger::Error << "K600PositionInterface: How many Leds?????" << Logger::endl;
	Logger::log() << Logger::Error << "K600PositionInterface: Prepare for Segfault :-)" << Logger::endl;
	return false;
      }
  }
#endif
}//namespace

