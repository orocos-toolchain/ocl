/***************************************************************************

                     datasender.hpp -  description
                           -------------------
    begin                : Wed August 9 2006
    copyright            : (C) 2006 Bas Kemper
                           (C) 2007 Ruben Smits //Changed subscription structure
    email                : kst@baskemper.be
                           first dot last at mech dot kuleuven dot be

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

#ifndef ORO_COMP_TCP_DATASENDER
#define ORO_COMP_TCP_DATASENDER

#include <rtt/Activity.hpp>
#include <rtt/os/Mutex.hpp>
#include <rtt/Property.hpp>

using RTT::os::Mutex;
using RTT::base::PropertyBase;
using RTT::Property;
using RTT::PropertyBag;

namespace RTT
{
    class SocketMarshaller;
}
namespace OCL{

    namespace TCP{
        using namespace RTT;
        class TcpReportingInterpreter;
        class Socket;

        /**
         * This class manages the connection with one single client. It is
         * responsible for sending data to the client and managing the
         * state of the client.
         *
         * It has a thread responsible for reading data from the socket.
         */
        class Datasender
            : public RTT::Activity
        {
        private:
            os::Mutex lock;
            TcpReportingInterpreter* interpreter;
            void checkbag(const PropertyBag &v);
            void writeOut(base::PropertyBase* v);
            void writeOut(const PropertyBag &v);
            Socket* os;
            OCL::TcpReporting* reporter;
            unsigned long long limit;
            unsigned long long curframe;
            bool silenced;
            RTT::SocketMarshaller* marshaller;
            std::vector<std::string> subscriptions;

        public:
            Datasender(RTT::SocketMarshaller* marshaller, Socket* os);
            virtual ~Datasender();

            /**
             * Returns true if the connection of the datasender is valid,
             * false otherwise.
             */
            bool isValid() const;

            /**
             * Only frames up to frame <newlimit> will be processed.
             */
            void setLimit(unsigned long long newlimit);

            /**
             * Send data to the client.
             */
            void serialize(const PropertyBag &v);

            /**
             * Return the marshaller.
             */
            RTT::SocketMarshaller* getMarshaller() const;

            bool addSubscription(const std::string name );
            bool removeSubscription( const std::string& name );

            /**
             * Write a list of the current subscriptions to the socket.
             */
            void listSubscriptions();

            /**
             * Get socket associated with this datasender.
             */
            Socket& getSocket() const;

            /**
             * Data connection main loop
             */
            virtual void loop();

            /**
             * Try to finish this thread.
             */
            virtual bool breakloop();

            /**
             * Disable/enable output of data
             */
            void silence(bool newstate);

            /**
             * Remove this connection
             */
            void remove();
    };
}
}
#endif
