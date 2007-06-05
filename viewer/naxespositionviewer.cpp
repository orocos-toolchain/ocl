/***************************************************************************

						naxespositionviewer.cpp	
                       ---------------------------
    begin                : june 2006 
    copyright            : (C) 2006
    email                : Erwin.Aertbelien@mech.kuleuven.ac.be
 

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

#include <rtt/Logger.hpp>

#ifndef _REENTRANT
#define _REENTRANT
#endif
#include <ace/Reactor.h>
#include <ace/Svc_Handler.h>
#include <ace/Acceptor.h>
#include <ace/Synch.h>
#include <ace/SOCK_Acceptor.h>
#include <ace/SOCK_Connector.h>
#include <ace/INET_Addr.h>
#include <ace/Log_Msg.h>

#include <math.h>
#include <list>
#include <vector>
#include <iostream>

#include <ace/Select_Reactor_Base.h>

namespace OCL {

	using namespace RTT;
	using namespace std;

/**
 * Singlethreaded reactive server that can handle multiple clients.
 */

#define MAXNROFJOINTS 32

/**
 * (Machine dependend) message that is send by this server.
 */
struct DataRecord {
	int    nrofjoints;       // number of joints.
    double jointvalues[MAXNROFJOINTS];  // maximally 32 joint values
    bool   stop;             // when true, this is a request to stop

    DataRecord() {
        for (int i=0;i<MAXNROFJOINTS;++i) jointvalues[i] = 0.0;
        stop = false;
    }
};



class ClientHandler:
	public ACE_Svc_Handler <ACE_SOCK_STREAM,ACE_NULL_SYNCH>
{
	typedef std::list<ClientHandler*> ClientHandlers;
	static std::list<ClientHandler*> clients;
public:
	static ACE_Reactor* reactor_instance;

	static void send_data(const std::vector<double>& q, bool stop) {
		DataRecord data;
		data.nrofjoints = q.size();
		for (unsigned int i=0;i<q.size();++i) {
			data.jointvalues[i] = q[i];
		}
		data.stop = stop;
  		//log(Info) << "(Viewer) sending " << data.jointvalues[2] << " for thirth joint " << endlog();
		for (ClientHandlers::iterator it=clients.begin();it!=clients.end();it++) {
			(*it)->peer().send_n(&data,sizeof(data));
		}
	} 

	static void delete_all() {
		for (ClientHandlers::iterator it=clients.begin();it!=clients.end();it++) {
			delete (*it);
			*it = 0;
  			log(Info) << "(Viewer) Connection closed" << endlog();
		}
	} 


	virtual int open(void*) {
  		log(Info) << "(Viewer)Connection established" << endlog();
	    if (reactor_instance==0) {
  			log(Info) << "(Viewer) Programming error : reactor_instance should not be 0" << endlog();
		}
		reactor_instance->register_handler(this, ACE_Event_Handler::READ_MASK);
		clients.push_back(this);
        return 0;
	}
	virtual int	handle_close (ACE_HANDLE, ACE_Reactor_Mask) {
		for (ClientHandlers::iterator it=clients.begin();it!=clients.end();it++) {
			if (*it==this) {
				clients.erase(it);
				break;
			}
		}
  		log(Info) << "(Viewer)Connection closed" << endlog();
		delete this;
        return 0;
	}
};

typedef ACE_Acceptor<ClientHandler,ACE_SOCK_ACCEPTOR> ClientAcceptor;

std::list<ClientHandler*> ClientHandler::clients;
ACE_Reactor* ClientHandler::reactor_instance = 0;

	
NAxesPositionViewer::NAxesPositionViewer(const std::string& name,const std::string& propertyfilename)
  : TaskContext(name),
    _propertyfile(propertyfilename),
	portnumber("portnumber","Port number to listen to for clients"),
	num_axes("numaxes","Number of axes to observe"),
    seperate_ports("seperate_ports","If it is true the input is of the form name0...nameN, otherwise it is a std::vector"),
    port_name("port_name","base name of the input"),
	clientacceptor(0),
	state(0)
{
  log(Debug) << "Entering NAxesPositionViewer::NAxesPositionViewer" << endlog();
  /**
   * initializing properties.
   */
  portnumber.value() = 9999;
  num_axes.value()   = 6;

  /**
   * Adding properties
   */
  properties()->addProperty( &portnumber );
  properties()->addProperty( &num_axes );
  properties()->addProperty( &seperate_ports);
  properties()->addProperty( &port_name);
 
  if (!marshalling()->readProperties(_propertyfile)) {
    log(Error) << "Failed to read the property file, continue with default values." << endlog();
  }
  _num_axes = num_axes.value();

  log(Debug) << "creating dataport(s) with base name : " << port_name.value() << endlog();
  /**
   * Creating and adding the data-ports
   */
  if (seperate_ports.value()) {
    seperateValues.resize(_num_axes);
    jointvec.resize(_num_axes);
    for (int i=0;i<_num_axes;++i) {
        char buf[80];
        sprintf(buf,"%s%d",port_name.value().c_str(),i);
        seperateValues[i]  = new ReadDataPort<double>(buf);
        ports()->addPort(seperateValues[i]);
    }
  } else {
    vectorValue = new RTT::ReadDataPort<std::vector<double> >(port_name.value()); 
    jointvec.resize(_num_axes);
    ports()->addPort(vectorValue);
  }

  log(Debug) << "Leaving NAxesPositionViewer::NAxesPositionViewer" << endlog();
}

NAxesPositionViewer::~NAxesPositionViewer()
{
//	if (clientacceptor!=0)
//		shutdown();
}

/**
 *  This function contains the application's startup code.
 *  Return false to abort startup.
 **/
bool NAxesPositionViewer::startup() {
	state=1;
    return true;
}
                   
/**
 * This function is periodically called.
 */
void NAxesPositionViewer::update() {
	// ACE_DEBUG( (LM_INFO,"%t update\n"));
	if (state==1) {
  	  log(Info) << "(Viewer) startup()" << endlog();
  	  ACE_INET_Addr addr(portnumber);
      ClientHandler::reactor_instance = new ACE_Reactor();
      clientacceptor=new ClientAcceptor(addr,ClientHandler::reactor_instance);
	  state=2;
	}
	if (state==2) {
        if (seperate_ports.value()) {
		    for (unsigned int i=0;i<jointvec.size();i++) {
			    jointvec[i] = seperateValues[i]->Get();
		    }
        } else {
            unsigned int minsize = jointvec.size();
            if (vectorValue->Get().size() < minsize) 
                minsize=vectorValue->Get().size();
    	    for (unsigned int i=0;i<minsize;i++) {
			    jointvec[i] = vectorValue->Get()[i];
		    }
        }
		ACE_Time_Value dt; 
		dt.set(0.01);
		ClientHandler::reactor_instance->handle_events(dt);
		ClientHandler::send_data(jointvec,false);
	}
} 

/**
 * This function is called when the task is stopped.
 */
void NAxesPositionViewer::shutdown() {
    log(Info) << "(Viewer) shutdown()" << endlog();
	ClientHandler::send_data(jointvec,true);
	ClientHandler::delete_all();
	delete (ClientAcceptor*)clientacceptor;
	delete ClientHandler::reactor_instance;
	ClientHandler::reactor_instance=0;
	clientacceptor=0;
}


} // end of namespace Orocos

