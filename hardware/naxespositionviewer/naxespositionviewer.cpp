/***************************************************************************
 tag: Erwin Aertbelien May 2006
                           -------------------
    begin                : Mon January 19 2004
    copyright            : (C) 2004 Peter Soetens
    email                : Erwin.Aertbelien@mech.kuleuven.ac.be
 
 based on the work of Johan Rutgeerts in LiASHardware.cpp

 ***************************************************************************
 *   This library is free software; you can redistribute it and/or         *
 *   modify it under the terms of the GNU Lesser General Public            *
 *   License as published by the Free Software Foundation; either          *
 *   version 2.1 of the License, or (at your option) any later version.    *
 *                                                                         *
 *   This library is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU     *
 *   Lesser General Public License for more details.                       *
 *                                                                         *
 *   You should have received a copy of the GNU Lesser General Public      *
 *   License along with this library; if not, write to the Free Software   *
 *   Foundation, Inc., 59 Temple Place,                                    *
 *   Suite 330, Boston, MA  02111-1307  USA                                *
 *                                                                         *
 ***************************************************************************/ 

#include "naxespositionviewer.hpp"

#include <execution/GenericTaskContext.hpp>
//#include <corelib/NonPreemptibleActivity.hpp>
//#include <execution/TemplateFactories.hpp>
//#include <execution/BufferPort.hpp>
//#include <corelib/Event.hpp>
#include <corelib/Logger.hpp>
#include <corelib/Attribute.hpp>
#include <execution/DataPort.hpp>
//#include <iostream>

#include <ace/SOCK_Acceptor.h>
#include <ace/SOCK_Connector.h>
#include <ace/INET_Addr.h>
#include <ace/Log_Msg.h>
#include <math.h>



namespace Orocos {

	using namespace RTT;
	using namespace std;


#define MAXNROFJOINTS 32
struct DataRecord {
	int    nrofjoints;       // number of joints.
    double jointvalues[MAXNROFJOINTS];  // maximally 32 joint values
    bool   stop;             // when true, this is a request to stop

    DataRecord() {
        for (int i=0;i<MAXNROFJOINTS;++i) jointvalues[i] = 0.0;
        stop = false;
    }
};




NAxesPositionViewer::NAxesPositionViewer(const std::string& name,const std::string& propertyfilename)
  : GenericTaskContext(name),
    _propertyfile(propertyfilename),
	portnumber("PortNumber","Port number to listen to for clients"),
	num_axes("NumAxes","Number of axes to observe"),
	stream(0)
{
  Logger::log() << Logger::Debug << "Entering NAxesPositionViewer::NAxesPositionViewer" << Logger::endl;
  /**
   * initializing properties.
   */
  portnumber.value() = 9999;
  num_axes.value()   = 6;

  /**
   * Adding properties
   */
  attributes()->addProperty( &portnumber );
  attributes()->addProperty( &num_axes );
 
  if (!readProperties(_propertyfile)) {
    Logger::log() << Logger::Error << "Failed to read the property file, continue with default values." << Logger::endl;
  }
  _num_axes = num_axes.value();
  /*
   *  Command Interface
   *
  typedef LiASnAxesVelocityController MyType;
  TemplateCommandFactory<MyType>* cfact = newCommandFactory( this );
  cfact->add( "startAxis",         command( &MyType::startAxis,         &MyType::startAxisCompleted, "start axis, initializes drive value to zero and starts updating the drive-value with the drive-port (only possible if axis is unlocked)","axis","axis to start" ) );
  this->commands()->registerObject("this", cfact);
  */
 
  /**
   * Method interface
   * 
  TemplateMethodFactory<MyType>* cmeth = newMethodFactory( this );
  cmeth->add( "isDriven",         method( &MyType::isDriven,  "checks wether axis is driven","axis","axis to check" ) );
  this->methods()->registerObject("this", cmeth);
  */


  Logger::log() << Logger::Debug << "creating dataports" << Logger::endl;
  /**
   * Creating and adding the data-ports
   */
  positionValue.resize(_num_axes);
  for (int i=0;i<_num_axes;++i) {
      char buf[80];
      sprintf(buf,"positionValue%d",i);
      positionValue[i]  = new ReadDataPort<double>(buf);
      ports()->addPort(positionValue[i]);
  }

  /**
   * Adding the events :
   *
   events()->addEvent( "driveOutOfRange", &driveOutOfRange );
   events()->addEvent( "positionOutOfRange", &positionOutOfRange );

   **
	* Connecting EventC to Events making c++-emit possible
	*
	driveOutOfRange_eventc = events()->setupEmit("driveOutOfRange").arg(driveOutOfRange_axis).arg(driveOutOfRange_value);
	positionOutOfRange_eventc = events()->setupEmit("positionOutOfRange").arg(positionOutOfRange_axis).arg(positionOutOfRange_value);
   */
  Logger::log() << Logger::Debug << "Leaving NAxesPositionViewer::NAxesPositionViewer" << Logger::endl;
}

NAxesPositionViewer::~NAxesPositionViewer()
{
}

/**
 *  This function contains the application's startup code.
 *  Return false to abort startup.
 **/
bool NAxesPositionViewer::startup() {
  ACE_INET_Addr addr(portnumber,"localhost");
  ACE_SOCK_Acceptor acceptor(addr);
  stream = new ACE_SOCK_Stream();
  acceptor.accept(*stream); 
  return true;
}
                   
/**
 * This function is periodically called.
 */
void NAxesPositionViewer::update() {
	DataRecord data;
	for (int i=0;i< _num_axes;i++) {
		data.jointvalues[i] = positionValue[i]->Get();
	}
	stream->send_n(&data,sizeof(data));
}
 

/**
 * This function is called when the task is stopped.
 */
void NAxesPositionViewer::shutdown() {
	stream->close();
	delete stream;
	stream=0;
    //writeProperties(_propertyfile);
}


} // end of namespace Orocos

