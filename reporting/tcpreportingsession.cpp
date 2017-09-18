#include "tcpreportingsession.hpp"
#include <boost/bind.hpp>
#include <rtt/Logger.hpp>
#include <rtt/Property.hpp>
#include <boost/algorithm/string.hpp>

namespace OCL {
TcpReportingSession::TcpReportingSession(tcp::socket *socket)
    : frameLimit(0),
      sentFrames(0),
      silenced(false),
      socket(socket),
      ostream(&output_buffer),
      interpreter(this) {
  registerRead();
  getOstream() << "100 Orocos 1.0 TcpReporting Server 1.0" << std::endl;
  flushOstream();
}

TcpReportingSession::~TcpReportingSession() { delete socket; }

void TcpReportingSession::terminate() {
  if (socket->is_open()) {
    socket->shutdown(tcp::socket::shutdown_send);
    asio::error_code error;
    socket->close(error);
    if (error) {
      RTT::log(RTT::Error) << "error closing socket: " << error.message()
                           << RTT::endlog();
    }
  }
}

std::ostream &TcpReportingSession::getOstream() { return ostream; }

void TcpReportingSession::flushOstream() {
  if (output_buffer.size() > 0)
    asio::async_write(*socket, output_buffer.data(),
                      boost::bind(&TcpReportingSession::handleWrite, this,
                                  asio::placeholders::error(),
                                  asio::placeholders::bytes_transferred()));
}

bool TcpReportingSession::addSubscription(std::string itemName) {
  return subscriptions.insert(itemName).second;
}

bool TcpReportingSession::removeSubscription(std::string itemName) {
  return subscriptions.erase(itemName);
}

boost::unordered_set<std::string> &TcpReportingSession::getSubscriptions() {
  return subscriptions;
}

void TcpReportingSession::registerRead() {
  asio::async_read_until(*socket, input_buffer, '\n',
                         boost::bind(&TcpReportingSession::handleLine, this,
                                     asio::placeholders::error,
                                     asio::placeholders::bytes_transferred));
}

void TcpReportingSession::handleLine(const asio::error_code &error,
                                     std::size_t bytes_transferred) {
  if (error) {
    asio::error_code closing_error;
    socket->close(closing_error);
    if (closing_error)
      RTT::log(RTT::Error) << "error closing socket: "
                           << closing_error.message() << RTT::endlog();

  } else {
    std::istream is(&input_buffer);
    std::string line;
    line.resize(bytes_transferred);
    is.getline(&line[0], bytes_transferred, '\n');
    interpreter.processLine(line.substr(0, bytes_transferred - 1));
    registerRead();
  }
}

void TcpReportingSession::handleWrite(const asio::error_code &error,
                                      std::size_t bytes_transferred) {
  if (error) {
    terminate();
    RTT::log(RTT::Error) << "error writing data: " << error.message()
                         << RTT::endlog();
  } else {
    output_buffer.consume(bytes_transferred);
  }
}

void TcpReportingSession::writeItemUpdate(RTT::base::PropertyBase *prop) {
  if (subscriptions.count(prop->getName())) {
    if (dynamic_cast<RTT::Property<RTT::PropertyBag> *>(prop)) {
      // TODO: maybe implement?
      //      serialize(bag->value());
    } else {
      getOstream() << "202 " << prop->getName() << "\n";
      getOstream() << "205 " << prop->getDataSource() << "\n";
    }
  }
}

void TcpReportingSession::flush() { flushOstream(); }

void TcpReportingSession::serialize(RTT::base::PropertyBase *v) {
  if (shouldSendFrame()) {
  }
}

void TcpReportingSession::serialize(const RTT::PropertyBag &v) {
  lastSerializedPropertyBag = &v;

  if (shouldSendFrame()) {
    for (int i = 0; i < v.size(); ++i) {
      writeItemUpdate(v.getItem(i));
    }
    getOstream() << "203 " << sentFrames << " -- end of frame" << std::endl;
    sentFrames++;
    if (sentFrames > frameLimit && frameLimit != 0) {
      getOstream() << "204 Limit reached" << std::endl;
    }
  }
}
}
