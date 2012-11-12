/***************************************************************************

                       TcpReporting.cpp -  TCP reporter
                           -------------------
    begin                : Fri Aug 4 2006
    copyright            : (C) 2006 Bas Kemper
                           2007-2008 Ruben Smits
    email                : kst@ <my name> .be

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

#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/types.h>
#include <errno.h>

#include "TcpReporting.hpp"
#include <rtt/Activity.hpp>
#include <rtt/Logger.hpp>
#include <rtt/os/Mutex.hpp>
#include "socket.hpp"
#include "socketmarshaller.hpp"

using RTT::Logger;
using RTT::os::Mutex;

#include "ocl/Component.hpp"
ORO_LIST_COMPONENT_TYPE(OCL::TcpReporting);

namespace OCL
{
    /**
     * ListenThread is a thread which waits for new incoming connections
     * from clients.
     */
    class ListenThread
        : public RTT::Activity
    {
        private:
            bool inBreak;
            static ListenThread* _instance;
            RTT::SocketMarshaller* _marshaller;
            unsigned short _port;
            bool _accepting;
            int _sock;

            bool listen()
            {
                _sock = ::socket(PF_INET, SOCK_STREAM, 0);
                if( _sock < 0 )
                {
                    Logger::log() << Logger::Error << "Socket creation failed." << Logger::endl;
                    return false;
                }

                struct sockaddr_in localsocket;
                struct sockaddr remote;
                int adrlen = sizeof(remote);

                localsocket.sin_family = AF_INET;
                localsocket.sin_port = htons(_port);
                localsocket.sin_addr.s_addr = INADDR_ANY;
                if( ::bind(_sock, (struct sockaddr*)&localsocket, sizeof(localsocket) ) < 0 )
                {
                    /* bind can fail when there is a legitimate server when a
                       previous run of orocos has crashed and the kernel does
                       not have freed the port yet. TRY_OTHER_PORTS can
                       select another port if the bind fails. */
                    #define TRY_OTHER_PORTS
                    // TODO: remove #define
                    #ifdef TRY_OTHER_PORTS
                    int i = 1;
                    int r = -1;
                    while( errno == EADDRINUSE && i < 5 && r < 0 )
                    {
                        localsocket.sin_port = htons(_port + i);
                        r = ::bind(_sock, (struct sockaddr*)&localsocket, sizeof(localsocket) );
                        i++;
                    }
                    if( r >= 0 )
                    {
                        Logger::log() << Logger::Info << "Port occupied, use port " << (_port+i-1) << " instead." << Logger::endl;
                    } else {
                    #endif
                    if( errno == EADDRINUSE )
                    {
                        Logger::log() << Logger::Error << "Binding of port failed: address already in use." << Logger::endl;
                    } else {
                        Logger::log() << Logger::Error << "Binding of port failed with errno " << errno << Logger::endl;
                    }
                    ::close(_sock);
                    return false;
                    #ifdef TRY_OTHER_PORTS
                    }
                    #endif
                }

                if( ::listen(_sock, 2) < 0 )
                {
                    Logger::log() << Logger::Info << "Cannot listen on socket" << Logger::endl;
                    ::close(_sock);
                    return true;
                }
                while(_accepting)
                {
                    int socket = ::accept( _sock, &remote,
                                           reinterpret_cast<socklen_t*>(&adrlen) );
                    if( socket == -1 )
                    {
                        return false;
                    }
                    if( _accepting )
                    {
                        Logger::log() << Logger::Info << "Incoming connection" << Logger::endl;
                        _marshaller->addConnection( new Orocos::TCP::Socket(socket) );
                    }
                }
                return true;
            }

            ListenThread( RTT::SocketMarshaller* marshaller, unsigned short port )
            : Activity(10), _marshaller(marshaller)
            {
                inBreak = false;
                removeInstance();
                _accepting = true;
                _port = port;
                Logger::log() << Logger::Info << "Starting server on port " << port << Logger::endl;
                this->Activity::start();
            }

        // This method should only be called when theadCreationLock is locked.
            void removeInstance()
            {
              if( _instance )
              {
                delete _instance;
              }
            }

      public:
          ~ListenThread()
          {
              _accepting = false;
          }

          virtual void loop()
          {
              if( !inBreak )
              {
                  if( !listen() )
                  {
                      Logger::log() << Logger::Error << "Could not listen on port " << _port << Logger::endl;
                  } else {
                      Logger::log() << Logger::Info << "Shutting down server" << Logger::endl;
                  }
              }
          }

          virtual bool breakLoop()
          {
              inBreak = true;
              _accepting = false;
              ::close( _sock );
              // accept still hangs until a new connection has been established
              int sock = ::socket(PF_INET, SOCK_STREAM, 0);
              if( sock > 0 )
              {
                  struct sockaddr_in socket;
                  socket.sin_family = AF_INET;
                  socket.sin_port = htons(_port);
                  socket.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
                  ::connect( sock, (struct sockaddr*)&socket, sizeof(socket) );
                  ::close( sock );
              }
              return true;
          }

          static void createInstance( RTT::SocketMarshaller* marshaller, unsigned short port = 3142 )
          {
            // The lock is needed to avoid problems when createInstance is called by two
            // different threads (which in reality should not occur very often).
            //ListenThread* _oinst = ListenThread::_instance;
            ListenThread::_instance = new ListenThread( marshaller, port );
            //delete _oinst;
          }

          static void destroyInstance()
          {
              ListenThread::_instance->breakLoop();
          }
    };
    ListenThread* ListenThread::_instance = 0;
}

namespace OCL
{
    TcpReporting::TcpReporting(std::string fr_name /*= "Reporting"*/)
        : ReportingComponent( fr_name ),
          port_prop("port","port to listen/send to",3142)
    {
        _finishing = false;
        this->properties()->addProperty( port_prop);
    }

    TcpReporting::~TcpReporting()
    {
    }

    const RTT::PropertyBag* TcpReporting::getReport()
    {
        makeReport2();
        return &report;
    }

    bool TcpReporting::configureHook(){
        port=port_prop.value();
        return true;
    }

    bool TcpReporting::startHook()
    {
        RTT::Logger::In in("TcpReporting::startup");
        fbody = new RTT::SocketMarshaller(this);
        this->addMarshaller( 0, fbody );
        ListenThread::createInstance( fbody, port );
        return ReportingComponent::startHook();
    }

    void TcpReporting::stopHook()
    {
        _finishing = true;
        ListenThread::destroyInstance();
        fbody->shutdown();
        ReportingComponent::stopHook();
        this->removeMarshallers();
    }
}
