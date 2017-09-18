#ifndef TCPREPORTINGSESSION_HPP
#define TCPREPORTINGSESSION_HPP

#include <asio.hpp>
#include <asio/error_code.hpp>
#include <iostream>
#include <rtt/marsh/MarshallInterface.hpp>
#include <rtt/PropertyBag.hpp>
#include <boost/unordered_set.hpp>
#include <string.h>

#include "command.hpp"

namespace OCL {

using asio::ip::tcp;
class TcpReportingSession : public RTT::marsh::MarshallInterface {
 public:
  TcpReportingSession(tcp::socket* socket);
  ~TcpReportingSession();
  void terminate();
  std::ostream& getOstream();
  void flushOstream();
  bool addSubscription(std::string itemName);
  bool removeSubscription(std::string itemName);
  boost::unordered_set<std::string> &getSubscriptions();
  const RTT::PropertyBag* lastSerializedPropertyBag;
  unsigned long frameLimit;
  unsigned long sentFrames;
  bool silenced;

  virtual void flush();
  virtual void serialize(RTT::base::PropertyBase*);
  virtual void serialize(const RTT::PropertyBag& v);

 private:
  tcp::socket* socket;
  asio::streambuf input_buffer;
  asio::streambuf output_buffer;
  std::ostream ostream;

  TcpReportingInterpreter interpreter;

  boost::unordered_set<std::string> subscriptions;

  void registerRead();
  void handleLine(const asio::error_code& error, std::size_t bytes_transferred);
  void handleWrite(const asio::error_code& error,
                   std::size_t bytes_transferred);

  void writeItemUpdate(RTT::base::PropertyBase* prop);
  inline bool shouldSendFrame() {
    return !subscriptions.empty() && (frameLimit == 0 || sentFrames <= frameLimit) &&
           !silenced && socket->is_open();
  }
};
}

#endif  // TCPREPORTINGSESSION_HPP
