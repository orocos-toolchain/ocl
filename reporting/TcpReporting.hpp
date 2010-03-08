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

namespace OCL
{
    namespace TCP
    {
        class TcpReportingInterpreter;
        class Socket;
    }


    /**
       \brief A component which writes data reports to a tcp/ip
       socket. It can serve different clients. It uses a ASCI-based
       protocol.

       \section usage Usage
       \subsection authent Authentication of the client:
       The server accepts different kinds of commands. Before these
       commands are available for the client, the client has to
       authenticate itself.
       - \b Send:
         \verbatim "VERSION 1.0\n" \endverbatim
       - \b Receive:
         - if succesfull: \verbatim "101 OK\n" \endverbatim
         - if failed:     \verbatim "106 not supported\n" \endverbatim

       \subsection help Getting Help:
       The client can get the available commands
       - \b Send:
         \verbatim "HELP\n" \endverbatim
       - \b Receive:
         \verbatim
         "Use HELP <command>\n
          CommandName1\n
          CommandName2\n
          ...
          CommandNameN\n."
         \endverbatim
       .
       and the syntax for each command.
       - \b Send:
         \verbatim"HELP CommandName\n" \endverbatim
       - \b Receive:
         \verbatim
         "Name: CommandName\n
          Usage: CommandName CommandSyntax\n"
         \endverbatim

       \subsection headers Getting a list of available data:
       The client can get the names of all the available data.
       - \b Send:
         \verbatim "HEADERS\n" \endverbatim
       - \b Receive:
         \verbatim
         "305 DataName1\n
          305 DataName2\n
          ...
          305 DataNameN\n
          306 End of list\n"
         \endverbatim

       \subsection subscribe Subscribe to Data:
       The client has to send the server the names of the available
       data he wants to get. Only the subscribed data will be send to
       the client.
       - \b Send:
         \verbatim "SUBSCRIBE DataNameX\n" \endverbatim
       - \b Receive:
         - if succesfull: \verbatim "302 DataNameX\n" \endverbatim
         - if failed:     \verbatim "301 DataNameX\n" \endverbatim

       \subsection unsubscribe Unsubscribe to Data:
       The client can cancel a subscription.
       - \b Send:
         \verbatim "UNSUBSCRIBE DataNameX\n" \endverbatim
       - \b Receive:
         - if succesfull: \verbatim "303 DataNameX\n" \endverbatim
         - if failed:     \verbatim "304 DataNameX\n" \endverbatim

       \subsection subs Getting a list of the subscriptions:
       The client can ask for the subscriptions he has made.
       - \b Send:
         \verbatim "SUBS\n" \endverbatim
       - \b Receive:
         \verbatim
         "305 DataNameX1\n
          305 DataNameX2\n
          ...
          305 DataNameXN\n
          306 End of list\n"
         \endverbatim

       \subsection silence Start/stop streaming the data:
       The client can start and stop the streaming of the subscribed
       data.
       - \b Send:
         -To start: \verbatim "SILENCE OFF\n" \endverbatim
         -To stop:  \verbatim "SILENCE ON\n" \endverbatim
       - \b Receive:
         \verbatim "107 SILENCE ON/OFF\n" \endverbatim

       \subsection quit Close the connection with the server:
       The client can close the connection with the server.
       - \b Send:
         \verbatim "QUIT\n" \endverbatim
         or
         \verbatim "EXIT\n" \endverbatim
       - \b Receive:
         \verbatim "104 Bye Bye\n" \endverbatim

       \subsection error When an error occures:
       When an error occurs because of wrong syntax the server will
       answer with an error message.
       - \b Send: wrong command syntax
       - \b Receive:
         \verbatim "102 Syntax: CommandNameX CommandSyntaxX\n" \endverbatim

       \subsection streaming The streaming data:
       When the streaming is started the server will send the
       following message at each timeframe.
       - \b Receive:
         \verbatim
         "201 framenr --- begin of frame\n
          202 DataNameX1\n
          205 DataValueX1\n
          202 DataNameX2\n
          205 DataValueX2\n
          ...
          202 DataNameXN\n
          205 DataValueXN\n
          203 framenr --- end of frame\n"
         \endverbatim
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
        unsigned int port;
        RTT::Property<unsigned int> port_prop;
    protected:
        /**
         * marsh::MarshallInterface
         */
        RTT::SocketMarshaller* fbody;

    public:
        /**
         * Create a reporting component which starts up a server
         *
         * @param fr_name   Name of the TCP reporting component.
         * @param port      Port to listen on.
         */
        TcpReporting(std::string fr_name = "ReportingComponent");
        ~TcpReporting();

        bool configureHook();
        bool startHook();

        void stopHook();

        /**
         * Return a property bag.
         */
        const RTT::PropertyBag* getReport();
    };

}

#endif
