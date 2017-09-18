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

#include "TcpReporting.hpp"
#include <rtt/Activity.hpp>
#include <rtt/Logger.hpp>
#include <rtt/Component.hpp>
#include <boost/bind.hpp>
#include <iostream>
#include "EmptyMarshaller.hpp"

ORO_LIST_COMPONENT_TYPE(OCL::TcpReporting);

namespace OCL {

void TcpReporting::registerAccept() {
  pending_socket = new tcp::socket(io_service);
  acceptor.async_accept(*pending_socket,
                        boost::bind(&TcpReporting::handleAccept, this,
                                    asio::placeholders::error));
}

void TcpReporting::handleAccept(const asio::error_code& error) {
  if (error) {
    delete pending_socket;
    stop();
  } else {
    TcpReportingSession* session = new TcpReportingSession(pending_socket);
    addMarshaller(session, session);
    sessions.push_back(session);
    registerAccept();
  }
}

TcpReporting::TcpReporting(std::string fr_name /*= "Reporting"*/)
    : ReportingComponent(fr_name), acceptor(io_service), port(3142) {
  addProperty("port", port);
  addMarshaller(0, new RTT::EmptyMarshaller());
}

TcpReporting::~TcpReporting() { delete pending_socket;}

bool TcpReporting::startHook() {
  tcp::endpoint endpoint(tcp::v4(), port);
  asio::error_code error;

  acceptor.open(endpoint.protocol(), error);
  if (error) {
    RTT::log(RTT::Error) << "failed to open acceptor: " << error.message()
                         << RTT::endlog();
    return false;
  }

  acceptor.set_option(tcp::acceptor::reuse_address(true));

  acceptor.bind(endpoint, error);
  if (error) {
    RTT::log(RTT::Error) << "failed to bind acceptor: " << error.message()
                         << RTT::endlog();
    return false;
  }

  acceptor.listen(asio::socket_base::max_connections, error);
  if (error) {
    RTT::log(RTT::Error) << "failed to listen acceptor: " << error.message()
                         << RTT::endlog();
    return false;
  }
  registerAccept();

  return ReportingComponent::startHook();
}

void TcpReporting::stopHook() {
  // Properly terminate the sessions
  for (int i = 0; i < sessions.size(); i++) {
    sessions[i]->terminate();
  }
  io_service.poll();  // we need this to invoke the session terminations

  // Free the sessions memory
  for (int i = 0; i < sessions.size(); i++) {
    delete sessions[i];
  }
  sessions.clear();

  // Close the acceptor
  acceptor.close();
  io_service.poll();
  io_service.reset();

  ReportingComponent::stopHook();
}

void TcpReporting::updateHook() {
  asio::error_code error;
  io_service.poll(error);
  if (error) {
    RTT::log(RTT::Error) << "error in updae hook" << error.message()
                         << RTT::endlog();
    stop();
  }
  ReportingComponent::updateHook();
}
}
