/***************************************************************************

                     socketmarshaller.hpp -  description
                           -------------------
    begin                : Mon August 7 2006
    copyright            : (C) 2006 Bas Kemper
    email                : kst@baskemper.be

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

#ifndef ORO_COMP_SOCKET_MARSHALLER
#define ORO_COMP_SOCKET_MARSHALLER

#include <rtt/Property.hpp>
#include <rtt/marsh/MarshallInterface.hpp>
#include <rtt/os/Mutex.hpp>
#include <list>

namespace OCL
{
class TcpReporting;
namespace TCP
{
    class Datasender;
    class Socket;
}
}

namespace RTT
{
    /**
     * marsh::MarshallInterface which sends data to multiple sockets.
     */
    class SocketMarshaller
        : public marsh::MarshallInterface
    {
        private:
            RTT::os::MutexRecursive lock;
            std::list<OCL::TCP::Datasender*> _connections;
            OCL::TcpReporting* _reporter;

        public:
            SocketMarshaller(OCL::TcpReporting* reporter);
            ~SocketMarshaller();
            virtual void flush();
            virtual void serialize(RTT::base::PropertyBase*);
            virtual void serialize(const PropertyBag &v);
            void addConnection(OCL::TCP::Socket* os);
            void removeConnection(OCL::TCP::Datasender* sender);
            void closeAllConnections();
            void shutdown();
            OCL::TcpReporting* getReporter() const;
    };
}
#endif
