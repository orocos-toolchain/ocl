/***************************************************************************

                     socketmarshaller.cpp -  description
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

#include <rtt/Logger.hpp>
#include <rtt/Property.hpp>
#include <rtt/base/PropertyIntrospection.hpp>
#include <rtt/os/Mutex.hpp>
#include "TcpReporting.hpp"
#include "socketmarshaller.hpp"
#include "datasender.hpp"

using RTT::Logger;

namespace RTT
{
        SocketMarshaller::SocketMarshaller(OCL::TcpReporting* reporter)
            : _reporter(reporter)
        {
        }

        SocketMarshaller::~SocketMarshaller()
        {
            closeAllConnections();
        }

        void SocketMarshaller::addConnection(OCL::TCP::Socket* os)
        {
            lock.lock();
            OCL::TCP::Datasender* conn = new OCL::TCP::Datasender(this, os);
            _connections.push_front( conn );
            conn->Activity::start();
            lock.unlock();
        }

        void SocketMarshaller::closeAllConnections()
        {
            // TODO: locking, proper connection shutdown
            while( !_connections.empty() )
            {
                removeConnection( _connections.front() );
            }
        }

        void SocketMarshaller::flush()
            {}

        void SocketMarshaller::removeConnection(OCL::TCP::Datasender* sender)
        {
            lock.lock();
            _connections.remove( sender );
            sender->breakLoop();
            delete sender;
            lock.unlock();
        }

        OCL::TcpReporting* SocketMarshaller::getReporter() const
        {
            return _reporter;
        }

        void SocketMarshaller::serialize(RTT::base::PropertyBase*)
        {
            // This method is pure virtual in the parent class.
            Logger::log() << Logger::Error << "Unexpected call to SocketMarshaller::serialize" <<
                    Logger::endl;
        }

        void SocketMarshaller::serialize(const PropertyBag &v)
        {
            lock.lock();
            // TODO: sending data does not run in parallel!
            for( std::list<OCL::TCP::Datasender*>::iterator it = _connections.begin();
                 it != _connections.end(); )
            {
                if( (*it)->isValid() )
                {
                    (*it)->serialize(v);
                    it++;
                } else {
                    OCL::TCP::Datasender* torm = *it;
                    it++;
                    removeConnection( torm );
                }
            }
            lock.unlock();
        }

        void SocketMarshaller::shutdown()
        {
            closeAllConnections();
        }
}
