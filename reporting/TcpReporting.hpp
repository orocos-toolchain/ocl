/***************************************************************************

                       TcpReporting.hpp -  TCP reporter
                           -------------------
    begin                : Fri Aug 4 2006
    copyright            : (C) 2006 Bas Kemper
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

#ifndef ORO_COMP_TCP_REPORTING_HPP
#define ORO_COMP_TCP_REPORTING_HPP

#include "ReportingComponent.hpp"
#include <iostream>

namespace RTT
{
    class SocketMarshaller;
}

namespace Orocos
{
    namespace TCP
    {
        class TcpReportingInterpreter;
        class Socket;
    }

    /**
     * A component which writes data reports to a console.
     */
    class TcpReporting
        : public ReportingComponent
    {
    private:
        /**
         * Flag to indicate that shutdown() has been called and
         * changes in the list of reported data should not be
         * reported to the client anymore.
         */
        bool _finishing;

    protected:
        /**
         * Marshaller
         */
        RTT::SocketMarshaller* fbody;

    public:
        /**
         * Create a reporting component which starts up a server
         *
         * @param fr_name   Name of the TCP reporting component.
         * @param port      Port to listen on.
         */
        TcpReporting(std::string fr_name = "ReportingComponent", unsigned short port = 3142);
        ~TcpReporting();

        bool startup();

        void shutdown();

        /**
         * Return a property bag.
         */
        const RTT::PropertyBag* getReport();
    };

}

#endif
